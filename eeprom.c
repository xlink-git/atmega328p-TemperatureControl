#include <stdio.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include "eeprom.h"

static EEMEM uint16_t target_temperature = DEFAULT_TARGET_TEMPERATURE;

int16_t eeprom_get_target_temperature(void)
{
    uint16_t temperature;
    
    temperature = eeprom_read_word(&target_temperature);
    
    printf("Target temperature : %d\r\n", temperature);
    
    if(temperature > MAX_TARGET_TEMPERATURE || temperature < MIN_TARGET_TEMPERATURE)
    {
        printf("Target temperature NOT in range : %d\r\n", temperature);
        temperature = DEFAULT_TARGET_TEMPERATURE;
        printf("Set default target temperature : %d\r\n", temperature);
        
        eeprom_update_target_temperature(temperature);
    }
    
    return temperature;
}

void eeprom_update_target_temperature(int16_t temperature)
{
    uint16_t read_v;
    
    if(temperature > MAX_TARGET_TEMPERATURE || temperature < MIN_TARGET_TEMPERATURE)
    {
        printf("Target temperature NOT in range : %d\r\n", temperature);
        temperature = DEFAULT_TARGET_TEMPERATURE;
        printf("Set default target temperature : %d\r\n", temperature);
    }
    
    eeprom_update_word(&target_temperature, temperature);
    
    read_v = eeprom_read_word(&target_temperature);
    
    printf("Update target temperature : %d / %d\r\n", temperature, read_v);
}