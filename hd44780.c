#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "hd44780.h"

static uint8_t hd44780_mask(uint8_t data)
{
	uint8_t mask = 0;
	
	mask |= data & 0x1 ? HD44780_DATA4 : 0;
	mask |= data & 0x2 ? HD44780_DATA5 : 0;
	mask |= data & 0x4 ? HD44780_DATA6 : 0;
	mask |= data & 0x8 ? HD44780_DATA7 : 0;
	
	return mask;
}

void hd44780_cmd(uint8_t command)
{
	HD44780_PORT &= ~(HD44780_DATA4 | HD44780_DATA5 | HD44780_DATA6 | HD44780_DATA7);
	HD44780_PORT |= hd44780_mask((command >> 4) & 0xf);
	HD44780_PORT &= ~HD44780_RS;
	HD44780_PORT |= HD44780_EN;
	_delay_us(300);
	HD44780_PORT &= ~HD44780_EN;
	_delay_us(300);
	HD44780_PORT &= ~(HD44780_DATA4 | HD44780_DATA5 | HD44780_DATA6 | HD44780_DATA7);
	HD44780_PORT |= hd44780_mask(command & 0xf);
	HD44780_PORT |= HD44780_EN;
	_delay_us(300);
	HD44780_PORT &= ~HD44780_EN;
	_delay_us(300);
}

void hd44780_data(uint8_t character)
{
	HD44780_PORT &= ~(HD44780_DATA4 | HD44780_DATA5 | HD44780_DATA6 | HD44780_DATA7);
	HD44780_PORT |= hd44780_mask((character >> 4) & 0xf);
	HD44780_PORT |= HD44780_RS;
	HD44780_PORT |= HD44780_EN;
	_delay_us(300);
	HD44780_PORT &= ~HD44780_EN;
	_delay_us(300);
	HD44780_PORT &= ~(HD44780_DATA4 | HD44780_DATA5 | HD44780_DATA6 | HD44780_DATA7);
	HD44780_PORT |= hd44780_mask(character & 0xf);
	HD44780_PORT |= HD44780_EN;
	_delay_us(300);
	HD44780_PORT &= ~HD44780_EN;
	_delay_us(300);
}

void hd44780_write(char *str)
{
	while (*str)
		hd44780_data(*str++);
}

void hd44780_gotoxy(uint8_t x, uint8_t y)
{
	switch (x) {
	case 0:
		hd44780_cmd(0x80 + (y & 0x1f));
		break;
	case 1:
		hd44780_cmd(0xc0 + (y & 0x1f));
		break;
	case 2:
		hd44780_cmd(0x94 + (y & 0x1f));
		break;
	case 3:
		hd44780_cmd(0xd4 + (y & 0x1f));
		break;
	default:
		break;
	}
}

void hd44780_init()
{
	HD44780_DDR |= HD44780_RS | HD44780_EN | HD44780_DATA4 | HD44780_DATA5 | HD44780_DATA6 | HD44780_DATA7;
	_delay_ms(100);			// power on
	
	hd44780_cmd(0x33);		// function set (initialize)
	_delay_ms(5);
	hd44780_cmd(0x33);		// function set (initialize)
	_delay_us(300);
	hd44780_cmd(0x32);		// function set (initialize)
	_delay_us(300);
	
	hd44780_cmd(0x28);		// function set
	_delay_us(300);
	hd44780_cmd(0x08);		// display off
	_delay_us(300);
	hd44780_cmd(0x01);		// clear display
	_delay_ms(3);
	hd44780_cmd(0x06);		// entry mode set
	_delay_us(300);
	hd44780_cmd(0x0c);		// display on
	hd44780_cmd(0x0c);		// display on
	_delay_us(300);
}
