// loop A:
// hits = 0
// loop B: (debounce de uma chave pressionada)
// v1 = ler chave
// delay de 5 ms
// v2 = ler chave
// se v1 <> 0 e v1 == v2
// hits = hits + 1
// senão
// hits = 0 e volta ao início do loop B
// se hits > 9, quebra loop B
// loop C: (debounce de uma chave liberada)
// v3 = ler chave
// delay de 5 ms
// v2 = ler chave
// se v3 == 0 e v3 == v2
// hits = hits + 1
// senão
// hits = 0 e volta ao início do loop C
// se hits > 9, quebra loop C
// se v1 <> 0
// retorna v1


#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "./print.h"
#include "./keypad.h"

/* scan keypad / decode matrix */

char scan_keypad()
{
	char keys[4][3] = {{'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};
	uint8_t i, j;
	
	for (j = 0; j < 3; j++) {
		if (j == 0) PORTD |= (1 << PD5);
		if (j == 1) PORTD |= (1 << PD6);
		if (j == 2) PORTD |= (1 << PD7);
		
		for (i = 0; i < 4; i++) {
			if (i == 0 && (PIND & (1 << PD0))) break;
			if (i == 1 && (PIND & (1 << PD1))) break;
			if (i == 2 && (PIND & (1 << PD2))) break;
			if (i == 3 && (PIND & (1 << PD3))) break;
		}
		PORTD &= ~((1 << PD5) | (1 << PD6) | (1 << PD7));

		if (i < 4)
			return keys[i][j];
	}
	
	return 0;
}


/* read keypad (blocking) and debounce keys */

char read_keypad()
{
    a: while(1) {
        int hits = 0;

        char v1;
        char v2;

        b: while(1) {
            v1 = scan_keypad();
            _delay_ms(5);
            v2 = scan_keypad();
            if(v1 != 0 && v2 == v1) {
                hits = hits + 1;
            } else {
                hits = 0;
            }

            if(hits > 9) {
                break;
            }
        }

        c: while(1) {
            char v3 = scan_keypad();
            _delay_ms(5);
            v2 = scan_keypad();
            if(v3 == 0 && v3 == v2) {
                hits = hits + 1;
            } else {
                hits = 0;
            }

            if(hits > 9) {
                break;
            }
        }

        if (v1 != 0) {
            return v1;
        }
    }

	return 0;
}

void keypad_init() {
    // configure PD0 ~ PD3 as inputs (with external pull-down resistors)
	DDRD &= ~((1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3));
	// configure PD5 ~ PD7 as outputs
	DDRD |= (1 << PD5) | (1 << PD6) | (1 << PD7);
}
