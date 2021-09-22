#define HD44780_DDR	DDRB
#define HD44780_PORT	PORTB

#define HD44780_RS	(1 << PB5)
#define HD44780_EN	(1 << PB4)
#define HD44780_DATA4	(1 << PB0)
#define HD44780_DATA5	(1 << PB1)
#define HD44780_DATA6	(1 << PB2)
#define HD44780_DATA7	(1 << PB3)

void hd44780_cmd(uint8_t command);
void hd44780_data(uint8_t character);
void hd44780_write(char *str);
void hd44780_gotoxy(uint8_t x, uint8_t y);
void hd44780_init();
