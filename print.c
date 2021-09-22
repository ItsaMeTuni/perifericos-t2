#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"
#include "print.h"
#include "./hd44780.h"

static int8_t *itoa(int32_t i, int8_t *s, int32_t base)
{
	int8_t c;
	int8_t *p = s;
	int8_t *q = s;
	uint32_t h;

	if (base == 16) {
		h = (uint32_t)i;
		do {
			*q++ = '0' + (h % base);
		} while (h /= base);
		if ((i >= 0) && (i < 16)) *q++ = '0';
		for (*q = 0; p <= --q; p++){
			(*p > '9')?(c = *p + 39):(c = *p);
			(*q > '9')?(*p = *q + 39):(*p = *q);
			*q = c;
		}
	} else {
		if (i >= 0) {
			do {
				*q++ = '0' + (i % base);
			} while (i /= base);
		} else {
			*q++ = '-';
			p++;
			do{
				*q++ = '0' - (i % base);
			} while (i /= base);
		}
		for (*q = 0; p <= --q; p++) {
			c = *p;
			*p = *q;
			*q = c;
		}
	}
	return s;
}

void hd44780_printint(uint32_t n)    
{
	int8_t buf[30];
	uint16_t i;

	itoa(n, buf, 10);
	i = 0;
	hd44780_write(buf);				
}

void uart_printint(uint32_t n)    
{
	int8_t buf[30];
	uint16_t i;

	char* s = itoa(n, buf, 10);
	i = 0;
	while (*s) {
		uart_tx(*s);
		s++;
	}			
}

void hd44780_printhex(uint32_t n)
{
	int8_t buf[30];
	uint16_t i;

	itoa(n, buf, 16);
	i = 0;
	hd44780_write(buf);
	
}

void hd44780_printfloat(float n)
{
	uint32_t v1, v2;
	
	v1 = (uint32_t)n;
	hd44780_printint(v1);			
	hd44780_print(".");
	v2 = ((n - (float)v1) * 100.0);
	// if (v2 < 100) hd44780_print("0");
	// if (v2 < 10) hd44780_print("0");
	hd44780_printint(v2);
}

void hd44780_print(char *s)
{
	hd44780_write(s);
	// while (*s) {
	// 	uart_tx(*s);
	// 	s++;
	// }
}
