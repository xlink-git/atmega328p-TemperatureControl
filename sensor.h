/* 
 * File:   sensor.h
 * Author: xlink
 *
 * Created on February 28, 2024, 11:10 PM
 */

#ifndef SENSOR_H
#define	SENSOR_H

#include "ds18b20.h"

#define TEMPERATURE_SENSOR_1    1
#define TEMPERATURE_SENSOR_2    2

#define ADC_SENSOR_1    6
#define ADC_SENSOR_2    7

#ifdef	__cplusplus
extern "C" {
#endif

uint8_t ds18b20_reset(uint8_t DS18B20_DQ);
void ds18b20_set9bitsres(uint8_t DS18B20_DQ);
int16_t ds18b20_gettemp(uint8_t DS18B20_DQ);

void adc_init(void);
uint16_t adc_read(uint8_t adc_pin);
int16_t get_temperature(uint8_t sensor_num);

#ifdef	__cplusplus
}
#endif

#endif	/* SENSOR_H */

