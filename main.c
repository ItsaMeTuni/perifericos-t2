#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "./uart.h"
#include "./hd44780.h"
#include "./adc.h"
#include "./print.h"
#include "./keypad.h"

#define RELAY_A PC0
#define RELAY_B PC1
#define RELAY_C PC2

#define LDR PC3
#define NTC PC4

//Lux and Thermistor
#define MAX_ADC_READING		1023
#define ADC_REF_VOLTAGE		5.0
#define REF_RESISTANCE		4700

#define SERIES_RES		4.7		// series resistance
#define NOM_RES			10.0		// nominal resistance (thermistor)
#define BETA			3455		// beta (thermistor)
#define NOM_TEMP		298.15		// nominal temperature (25 C)

#define MENU_MAIN 0
#define MENU_TIME 1
#define MENU_ALARM_TIME 2
#define MENU_LUX 3
#define MENU_TEMP 4

#define TIMER_CLK1		F_CPU / 256
#define IRQ_FREQ1		4				// irq frequency, in Hz

#define MENU_TIME_ON 0
#define MENU_TIME_OFF 1

#define MENU_TEMP_START 0
#define MENU_TEMP_END 1

#define MENU_LUX_START 0
#define MENU_LUX_END 1

#define HOUR 0
#define MINUTE 1
#define SECOND 2

typedef struct {
    int hours;
    int minutes;
    int seconds;
} Time;

char current_menu = MENU_MAIN;
char current_menu_section = 0;
char current_time_component = 0;

Time time = {.hours = 18, .minutes = 0, .seconds = 0};
char subSecond = 0;

Time time_on = {.hours = 18, .minutes = 0, .seconds = 5};
Time time_off = {.hours = 18, .minutes = 0, .seconds = 10};

int temp_start = 50;
int temp_end = 100;

int lux_start = 300;
int lux_end = 600;

int temp;
int lux;

float conversion_lux(uint16_t raw_adc)
{
	float vout, lux, rldr;
	
	vout = raw_adc * (ADC_REF_VOLTAGE / MAX_ADC_READING);
	rldr = (REF_RESISTANCE * (ADC_REF_VOLTAGE - vout)) / vout;
	lux = 500 / (rldr / 650);
	
	return lux;
}

float conversion_temp(uint16_t raw_adc)
{
	float vout, res, temp, celsius;
	
	vout = raw_adc * (ADC_REF_VOLTAGE / MAX_ADC_READING);
	/* convert voltage measured to resistance value */
	res = (vout * SERIES_RES) / (ADC_REF_VOLTAGE - vout);
	/* use resistance value in Steinhart and Hart equation, calculate temperature value in kelvin */
	temp =  1.0 / ((1.0 / NOM_TEMP) + ((log(res / NOM_RES)) / BETA));
	/* convert kelvin to celsius */
	celsius = temp - 273.15; // Converting kelvin to celsius
	
	return celsius;
}

void print_sensors() {

    hd44780_gotoxy(0, 0);
    hd44780_print("\nLux: ");

    hd44780_gotoxy(0, 14);
    hd44780_print("   ");

    hd44780_gotoxy(0, 14);
    hd44780_printint(lux);


    hd44780_gotoxy(1, 0);
    hd44780_print("\nTemperatura: ");

    hd44780_gotoxy(1, 14);
    hd44780_print("   ");

    hd44780_gotoxy(1, 14);
    hd44780_printint(temp);
}

void relay_on(char relay) {
    PORTC |= 1 << relay;
}

void relay_off(char relay) {
    PORTC &= ~(1 << relay);
}

void print_time(Time time) {
    if(time.hours < 10) {
        hd44780_print("0");
    }
    hd44780_printint(time.hours);

    hd44780_print(":");
    if(time.minutes < 10) {
        hd44780_print("0");
    }
    hd44780_printint(time.minutes);

    hd44780_print(":");
    if(time.seconds < 10) {
        hd44780_print("0");
    }
    hd44780_printint(time.seconds);
}

ISR(TIMER1_COMPA_vect) {
    subSecond++;
    if(subSecond >= 4) {
        time.seconds++;
        subSecond = 0;
    }

    if (time.seconds >= 60) {
        time.minutes++;
        time.seconds = 0;
    }

    if (time.minutes >= 60) {
        time.hours++;
        time.minutes = 0;
    }

    if (time.hours >= 24) {
        time.hours = 0;
    }
    
    //hd44780_cmd(0x01);
    //_delay_ms(3);

    adc_set_channel(3);
    lux = conversion_lux(adc_read());

    adc_set_channel(4);
    temp = conversion_temp(adc_read());    

    if (current_menu == MENU_MAIN) {
        hd44780_gotoxy(3, 12);
        print_time(time);

        print_sensors();
    }

    if(time.hours == time_on.hours
    && time.minutes == time_on.minutes
    && time.seconds == time_on.seconds) {
        relay_on(RELAY_A);
    }

    if(time.hours == time_off.hours
    && time.minutes == time_off.minutes
    && time.seconds == time_off.seconds) {
        relay_off(RELAY_A);
    }

    if(temp_end > temp_start) {
        if (temp > temp_start && temp < temp_end) {
            relay_on(RELAY_C);
        } else {
            relay_off(RELAY_C);
        }
    } else {
        if (temp > temp_end && temp < temp_start) {
            relay_on(RELAY_C);
        } else {
            relay_off(RELAY_C);
        }
    }

    if(lux_end > lux_start) {
        if (lux > lux_start && lux < lux_end) {
            relay_on(RELAY_B);
        } else {
            relay_off(RELAY_B);
        }
    } else {
        if (lux > lux_end && lux < lux_start) {
            relay_on(RELAY_B);
        } else {
            relay_off(RELAY_B);
        }
    }
}

int str_to_int(char* str, int digits) {
    int val = 0;
    int digit = 1;

    while(digit <= digits) {
        int charVal = *str - '0';

        // for some reason pow wasnt working correctly
        for(int i = 0; i < digits - digit; i++) {
            charVal *= 10;
        }

        val += charVal;

        digit++;
        str++;
    }

    return val;
}

void set_time_component(Time* time, char component, int val) {
    if (component == HOUR) {
        time->hours = val;
    } else if (component == MINUTE) {
        time->minutes = val;
    } else if (component == SECOND) {
        time->seconds = val;
    }

    if(time->hours > 23) {
        time->hours = 0;
    }

    if(time->minutes > 59) {
        time->minutes = 0;
    }

    if(time->seconds > 59) {
        time->seconds = 0;
    }
}

int main() 
{
    DDRB |= 1 << PB7;

    DDRC |= (1 << RELAY_A) | (1 << RELAY_B) | (1 << RELAY_C);

    TCNT1 = 0;
    TCCR1A = 0;
    TCCR1B = 0;
    OCR1A = (TIMER_CLK1 / IRQ_FREQ1) - 1;
    /* turn on CTC mode, timer 1*/
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12);				// clk / 256 (prescaler), clear-on-match
    
    TIMSK1 |= (1 << OCIE1A);

    hd44780_init();
    keypad_init();
    uart_init(57600, 1);
	adc_init();

    for (;;) {
        if (current_menu == MENU_MAIN) {
            //print_sensors();
        } else if (current_menu == MENU_TIME) {
            hd44780_gotoxy(0, 0);
            hd44780_print("Time");

            hd44780_gotoxy(1, 0);
            print_time(time);
        } else if (current_menu == MENU_ALARM_TIME) {
            hd44780_gotoxy(0, 0);
            hd44780_print("Time relay");

            hd44780_gotoxy(1, 0);
            hd44780_print(" On  ");
            print_time(time_on);

            hd44780_gotoxy(2, 0);
            hd44780_print(" Off ");
            print_time(time_off);

            hd44780_gotoxy(current_menu_section + 1, 0);
            hd44780_print(">");
        } else if (current_menu == MENU_LUX) {
            hd44780_print("Lux relay");
            hd44780_gotoxy(1, 0);
            hd44780_print(" Start ");
            hd44780_printint(lux_start);

            hd44780_gotoxy(2, 0);
            hd44780_print(" End   ");
            hd44780_printint(lux_end);

            hd44780_gotoxy(current_menu_section + 1, 0);
            hd44780_print(">");
        } else if (current_menu == MENU_TEMP) {
            hd44780_print("Temp relay");
            hd44780_gotoxy(1, 0);
            hd44780_print(" Start ");
            hd44780_printint(temp_start);

            hd44780_gotoxy(2, 0);
            hd44780_print(" End   ");
            hd44780_printint(temp_end);

            hd44780_gotoxy(current_menu_section + 1, 0);
            hd44780_print(">");
        }

		char key = read_keypad();

		if (key == '*') {
			current_menu = current_menu + 1;
            if (current_menu > 4) {
                current_menu = MENU_MAIN;
            }

            current_menu_section = 0;
            current_time_component = 0;
            
            hd44780_cmd(0x01);
            _delay_ms(3);
		} else if (key == '#') {
			current_menu_section = current_menu_section + 1;
            current_time_component = 0;
            if (current_menu == MENU_ALARM_TIME && current_menu_section > MENU_TIME_OFF) {
                current_menu_section = MENU_TIME_ON;
            }

            if (current_menu == MENU_TEMP && current_menu_section > MENU_TEMP_END) {
                current_menu_section = MENU_TEMP_START;
            }

            if (current_menu == MENU_LUX && current_menu_section > MENU_LUX_END) {
                current_menu_section = MENU_LUX_START;
            }
            
            hd44780_cmd(0x01);
            _delay_ms(3);
		} else {

            if (current_menu == MENU_TIME) {
                char buf[] = {key, read_keypad()};
                int val = str_to_int(buf, 2);
                
                set_time_component(&time, current_time_component, val);

                current_time_component++;
                if (current_time_component > SECOND) {
                    current_time_component = HOUR;
                }
            } else if (current_menu == MENU_ALARM_TIME) {
                char buf[] = {key, read_keypad()};
                int val = str_to_int(buf, 2);
                
                Time* alarm;
                if (current_menu_section == MENU_TIME_ON) {
                    alarm = &time_on;
                } else {
                    alarm = &time_off;
                }

                set_time_component(alarm, current_time_component, val);

                current_time_component++;
                if (current_time_component > SECOND) {
                    current_time_component = HOUR;
                }
            } else if (current_menu == MENU_LUX) {
                char buf[] = {key, read_keypad(), read_keypad()};

                int val = str_to_int(buf, 3);

                if (current_menu_section == MENU_LUX_START) {
                    lux_start = val;
                } else {
                    lux_end = val;
                }
            } else if (current_menu == MENU_TEMP) {
                char buf[] = {key, read_keypad(), read_keypad()};

                int val = str_to_int(buf, 3);

                if (current_menu_section == MENU_TEMP_START) {
                    temp_start = val;
                } else {
                    temp_end = val;
                }
            }



        }


	}

    // float ldrLux, ntc;
  
  	//uart_init(57600, 1);
	//adc_init();

    	// while(1) 
        // {
        //     char key = read_keypad();
        //     adc_set_channel(3);
        //     ldrLux = conversion_lux(adc_read());
        //     hd44780_gotoxy(0, 0);
        //     hd44780_print("\nLux: ");
        //     hd44780_gotoxy(0, 14);
        //     hd44780_printfloat(ldrLux);

        //     adc_set_channel(4);
        //     ntc = conversion_temp(adc_read());
        //     hd44780_gotoxy(1, 0);
        //     hd44780_print("\nTemperatura: ");
        //     hd44780_gotoxy(1, 14);
        //     hd44780_printfloat(ntc);
        // }
}