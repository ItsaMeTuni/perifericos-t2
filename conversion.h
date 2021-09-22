#include <stdint.h>

#define MAX_ADC_READING		1023
#define ADC_REF_VOLTAGE		5.0
#define REF_RESISTANCE		4700

#define SERIES_RES		4.7		    // series resistance
#define NOM_RES			10.0		// nominal resistance (thermistor)
#define BETA			3455		// beta (thermistor)
#define NOM_TEMP		298.15		// nominal temperature (25 C)

float conversion_lux(uint16_t raw_adc);
float conversion_temp(uint16_t raw_adc);