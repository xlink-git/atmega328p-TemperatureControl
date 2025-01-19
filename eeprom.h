/* 
 * File:   eeprom.h
 * Author: xlink
 *
 * Created on February 29, 2024, 8:25 PM
 */

#ifndef EEPROM_H
#define	EEPROM_H

#define DEFAULT_TARGET_TEMPERATURE  340 // 34.0
#define MAX_TARGET_TEMPERATURE  450     // 50.0
#define MIN_TARGET_TEMPERATURE  100     // 10.0

#ifdef	__cplusplus
extern "C" {
#endif

int16_t eeprom_get_target_temperature(void);
void eeprom_update_target_temperature(int16_t temperature);


#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

