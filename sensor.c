#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ds18b20.h"
#include "sensor.h"


uint8_t ds18b20_reset(uint8_t DS18B20_DQ) {
	uint8_t i;

	//low for 480us
	DS18B20_PORT &= ~_BV(DS18B20_DQ); //low
	DS18B20_DDR |= _BV(DS18B20_DQ); //output
	_delay_us(480);

	//release line and wait for 60uS
	DS18B20_DDR &= ~_BV(DS18B20_DQ); //input
	_delay_us(60);

	//get value and wait 420us
	i = (DS18B20_PIN & _BV(DS18B20_DQ));
	_delay_us(420);

    if(i) printf("DS18B20 reset ERROR : %d\r\n", DS18B20_DQ);
	//return the read value, 0=ok, 1=error
	return i;
}

/*
 * write one bit
 */
static void ds18b20_writebit(uint8_t bit, uint8_t DS18B20_DQ)
{
	//low for 1uS
	DS18B20_PORT &= ~_BV(DS18B20_DQ); //low
	DS18B20_DDR |= _BV(DS18B20_DQ); //output
	_delay_us(1);

	//if we want to write 1, release the line (if not will keep low)
	if(bit)
		DS18B20_DDR &= ~_BV(DS18B20_DQ); //input

	//wait 60uS and release the line
	_delay_us(60);
	DS18B20_DDR &= ~_BV(DS18B20_DQ); //input
}

/*
 * read one bit
 */
static uint8_t ds18b20_readbit(uint8_t DS18B20_DQ)
{
	uint8_t bit=0;

	//low for 1uS
	DS18B20_PORT &= ~_BV(DS18B20_DQ); //low
	DS18B20_DDR |= _BV(DS18B20_DQ); //output
	_delay_us(1);

	//release line and wait for 14uS
	DS18B20_DDR &= ~_BV(DS18B20_DQ); //input
	_delay_us(14);

	//read the value
	if(DS18B20_PIN & _BV(DS18B20_DQ))
		bit=1;

	//wait 45uS and return read value
	_delay_us(45);
	return bit;
}

/*
 * write one byte
 */
static void ds18b20_writebyte(uint8_t byte, uint8_t DS18B20_DQ)
{
	uint8_t i=8;
	while(i--){
		ds18b20_writebit(byte&1, DS18B20_DQ);
		byte >>= 1;
	}
}

/*
 * read one byte
 */
static uint8_t ds18b20_readbyte(uint8_t DS18B20_DQ)
{
	uint8_t i=8, n=0;
	while(i--){
		n >>= 1;
		n |= (ds18b20_readbit(DS18B20_DQ)<<7);
	}
	return n;
}

void ds18b20_set9bitsres(uint8_t DS18B20_DQ) 
{
	ds18b20_reset(DS18B20_DQ); //reset
	
	ds18b20_writebyte(DS18B20_CMD_SKIPROM, DS18B20_DQ); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_WSCRATCHPAD, DS18B20_DQ);
	ds18b20_writebyte(0x01, DS18B20_DQ);
	ds18b20_writebyte(0xE0, DS18B20_DQ);
	ds18b20_writebyte(0x1F, DS18B20_DQ);	// set 9 bits resolution
	
	ds18b20_reset(DS18B20_DQ); //reset
}

/*
 * get temperature
 * resolution : 0.1 degree
 */
int16_t ds18b20_gettemp(uint8_t DS18B20_DQ) 
{
	uint8_t temperature_l;
	uint8_t temperature_h;
	int16_t retd = 0;

#if DS18B20_STOPINTERRUPTONREAD == 1
	cli();
#endif

	ds18b20_reset(DS18B20_DQ); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM, DS18B20_DQ); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_CONVERTTEMP, DS18B20_DQ); //start temperature conversion

	while(!ds18b20_readbit(DS18B20_DQ)); //wait until conversion is complete

	ds18b20_reset(DS18B20_DQ); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM, DS18B20_DQ); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_RSCRATCHPAD, DS18B20_DQ); //read scratchpad

	//read 2 byte from scratchpad
	temperature_l = ds18b20_readbyte(DS18B20_DQ);
	temperature_h = ds18b20_readbyte(DS18B20_DQ);

#if DS18B20_STOPINTERRUPTONREAD == 1
	sei();
#endif

	//convert the 12 bit value obtained
	retd = (int16_t)((((temperature_h << 8 ) + temperature_l ) * 0.0625) * 10);
	
	if(retd < 0) retd = 0;

	return retd;
}

void adc_init(void)
{
    // AVCC with external capacitor at AREF pin
	ADMUX = _BV(REFS0);
    
    //enable ADC module, set prescalar of 128 which gives CLK/128 	
	ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);    
    
    // ADC6, ADC7 ? ADC ?? (QFP ????? ??)
//    DIDR0 = _BV(ADC4D) | _BV(ADC5D);
}

uint16_t adc_read(uint8_t adc_pin) 
{
    ADMUX &= 0xF0;
    ADMUX |= (adc_pin & 0x0F);         //ADC Channel Selection
    ADCSRA |= _BV(ADSC);             //ADC Start Conversion
    
    while (!(ADCSRA & _BV(ADIF)));    //When the conversion is complete, it returns to zero
    ADCSRA |= _BV(ADIF);
     
    return ADCW;                        //ADCL+ADCH 10bit 
}

//static double ntc_beta = 4113;
//static uint32_t ntc_resistor = 10000;

double steinhart_hart(double resistence)
{
	double temperature;

	/*
	Temperature (K) = 1 / (a + (b*ln(Rntc) + c.ln(Rntc)*ln(Rntc)*ln(Rntc)))

	a = 0.0011303
	b = 0.0002339
	c = 0.00000008863
	*/
	temperature = log(resistence);
    temperature = 1 / (0.0008474214 + (0.00025931143 * temperature) + (0.0000001247286 * temperature * temperature * temperature));
	
	//Convert in Celsius
	temperature = temperature - 273.15;

	return temperature;
}

int16_t get_temperature(uint8_t sensor_num)
{
    uint8_t adc_ch;
    uint16_t adc_val;
    double ntc_r, ntc_volt;
    
    if(sensor_num == TEMPERATURE_SENSOR_1) adc_ch = ADC_SENSOR_1;
    else adc_ch = ADC_SENSOR_2;
    
    adc_val = adc_read(adc_ch);
    ntc_volt = adc_val * 0.00488;
    
    ntc_r = (ntc_volt * 10000) / (5.0 - ntc_volt);
    
//    printf("adc : %d, ntc_r : %ld\r\n", adc_val, (uint32_t)ntc_r);
    
    double temperature = steinhart_hart(ntc_r);
    
//    double r = ntc_r / 10000.0;
//    double temperature = (1.0 / ((log(r) / 4113.0) + (1.0 / 298.15))) - 273.15;

    return (int16_t)(temperature * 10);
}