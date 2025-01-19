/*
 * Author: Kwon Taeyoung (xlink69@gmail.com)
 */

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "heater.h"
#include "sensor.h"
#include "eeprom.h"
#include "fnd.h"

FUSES = {
 .low = 0xFF, // LOW
 .high = 0xDA, // HIGH
 .extended = 0xFD
};

LOCKBITS = 0xFC; // {LB=NO_LOCK, BLB0=NO_LOCK, BLB1=NO_LOCK}

void uart_cmd_process(void)
{
    static uint8_t rx_count;
    uint16_t number;
    static char in[32];
    uint8_t data;
    
	
    if(rx_count >= 31) rx_count = 0;
    
    if(uart_rx(&data) != 0)
    {
        return;
    }
    
    uart_tx(data);      // echo
    
    in[rx_count] = data;
    if(data == '\r' || data == '\n')
    {
        in[rx_count] = 0;
        printf("\r\nInput command : %s\r\n", in);
        if(rx_count == 0) return;
        
        rx_count = 0;
    }
    else
    {
        rx_count++;
        return;
    }
    
    char *token;
    
    token = strtok(in, " \r\n");
    
    if(strcmp(in, "adc") == 0)
    {
        printf("\r\nadc6 : %d\r\n", adc_read(6));
        printf("adc7 : %d\r\n", adc_read(7));
        return;
    }
    if(strcmp(token, "fnd") == 0)
    {
        token = strtok(NULL, " \r\n");
        
        if(strcmp(token, "blinkon") == 0)
        {
            fnd_blink(1);
        }
        else if(strcmp(token, "blinkoff") == 0)
        {
            fnd_blink(0);
        }
        else
        {
            number = atoi(token);
            fnd_set_data(number);
        }
        return;
    }
    
    number = atoi(token);
    heater_set(number);
}

#define BUTTON_REPEAT_LIMIT 20  // 50 ms * 20 = 1 second

#define SW_DIR_PORT DDRC
#define SW_OUT_PORT PORTC
#define SW_IN_PORT  PINC
#define SW_SET      PINC3
#define SW_DOWN     PINC4
#define SW_UP       PINC5

static uint8_t temperature_set_mode;
static int16_t target_temperature;

void set_process(void)
{
    uint8_t count = 0;
    
    _delay_ms(20);
    
    if(bit_is_set(SW_IN_PORT, SW_SET)) return;
    
    printf("Set button pressed\r\n");
    
    while(1)
    {
        if(bit_is_set(SW_IN_PORT, SW_SET))
        {
            printf("Exit temperature set mode\r\n");
            temperature_set_mode = 0;
            fnd_blink(0);
            eeprom_update_target_temperature(target_temperature);
// Don't use PID control           
//            heater_clear_pid_values();
            break;
        }
        
        _delay_ms(100);
        count++;
        if(count >= 20)
        {
            printf("Set pressed for 2 seconds, enter set mode\r\n");
            fnd_blink(1);
            temperature_set_mode = 1;
            fnd_set_data(target_temperature);
            break;
        }
    }
    
    while(bit_is_clear(SW_IN_PORT, SW_SET));
    
    printf("Set button released\r\n");
    _delay_ms(20);
}

void up_process(void)
{
    _delay_ms(20);
    
    if(bit_is_set(SW_IN_PORT, SW_UP)) return;
    printf("Up button pressed\r\n");
    
    if(temperature_set_mode)
    {
        if(target_temperature < MAX_TARGET_TEMPERATURE) target_temperature++;
        
        fnd_set_data(target_temperature);
        
        uint8_t press_count = 0;
        while(1)
        {
            if(bit_is_set(SW_IN_PORT, SW_UP)) break;
            
            _delay_ms(50);
            if(++press_count > BUTTON_REPEAT_LIMIT) break;
        }
        
        while(press_count > BUTTON_REPEAT_LIMIT)
        {
            _delay_ms(100);
            if(bit_is_set(SW_IN_PORT, SW_UP)) break;
            
            if(target_temperature < MAX_TARGET_TEMPERATURE) target_temperature++;
        
            fnd_set_data(target_temperature);
        }
    }
    else
    {
        printf("Not setting mode, wait for UP button release\r\n");
    }
    
    while(bit_is_clear(SW_IN_PORT, SW_UP));
    
    printf("Up button released\r\n");
    _delay_ms(20);
}

void down_process(void)
{
    _delay_ms(20);
    
    if(bit_is_set(SW_IN_PORT, SW_DOWN)) return;
    printf("Down button pressed\r\n");
    
    if(temperature_set_mode)
    {
        if(target_temperature > MIN_TARGET_TEMPERATURE) target_temperature--;
        
        fnd_set_data(target_temperature);
        
        uint8_t press_count = 0;
        while(1)
        {
            if(bit_is_set(SW_IN_PORT, SW_DOWN)) break;
            
            _delay_ms(50);
            if(++press_count > BUTTON_REPEAT_LIMIT) break;
        }
        
        while(press_count > BUTTON_REPEAT_LIMIT)
        {
            _delay_ms(100);
            if(bit_is_set(SW_IN_PORT, SW_DOWN)) break;
            
            if(target_temperature > MIN_TARGET_TEMPERATURE) target_temperature--;
        
            fnd_set_data(target_temperature);
        }
    }
    else
    {
        printf("Not setting mode, wait for DOWN button release\r\n");
    }
    
    while(bit_is_clear(SW_IN_PORT, SW_DOWN));
    
    printf("Down button released\r\n");
    _delay_ms(20);
}

extern uint32_t timestamp_10ms;
int main(void)
{
    uint32_t ref_10ms, gap_10ms;
    int16_t cur_temperature, pre_temperature;
    
    // Set input
    SW_DIR_PORT &= !(_BV(SW_SET) | _BV(SW_DOWN) | _BV(SW_UP));
    // Set internal pull up
    SW_OUT_PORT |= (_BV(SW_SET) | _BV(SW_DOWN) | _BV(SW_UP));
    
	uart_init();
    printf("\r\nBooting\r\n\r\n");
    fnd_init();
    ds18b20_reset(DS18B20_DQ1);
    ds18b20_reset(DS18B20_DQ2);
    target_temperature = eeprom_get_target_temperature();
    
    adc_init();
    heater_pwm_init(10);
    sei();
    
    fnd_set_data(target_temperature);
    fnd_blink(1);
    _delay_ms(1000);
    fnd_blink(0);

    ref_10ms = timestamp_10ms;
    cur_temperature = get_temperature(TEMPERATURE_SENSOR_1);
    fnd_set_data(cur_temperature);
    
	while (1)
	{
		if(bit_is_clear(SW_IN_PORT, SW_SET))
        {
            set_process();
        }
        else if(bit_is_clear(SW_IN_PORT, SW_UP))
        {
            up_process();
        }
        else if(bit_is_clear(SW_IN_PORT, SW_DOWN))
        {
            down_process();
        }
        
        uart_cmd_process();
        
        cli();
        gap_10ms = timestamp_10ms - ref_10ms;
        sei();
        if(gap_10ms >= 50)
        {
            cli();
            ref_10ms = timestamp_10ms;
            sei();
            pre_temperature = cur_temperature;
            cur_temperature = get_temperature(TEMPERATURE_SENSOR_1);
            
            if(temperature_set_mode)
                heater_control(cur_temperature, cur_temperature, pre_temperature, gap_10ms * 10);
            else
                heater_control(target_temperature, cur_temperature, pre_temperature, gap_10ms * 10);
            
            if(!temperature_set_mode)
            {
                fnd_set_data(cur_temperature);
            }
        }
	}
}