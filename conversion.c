#include "./conversion.h"


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