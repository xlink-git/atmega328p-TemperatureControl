/* 
 * File:   ds18b20.h
 * Author: xlink
 *
 * Created on February 28, 2024, 10:26 PM
 */

#ifndef DS18B20_H
#define	DS18B20_H

#include <avr/io.h>

#ifdef	__cplusplus
extern "C" {
#endif

//setup connection
#define DS18B20_PORT PORTC
#define DS18B20_DDR DDRC
#define DS18B20_PIN PINC
    
#define DS18B20_DQ1 PC0
#define DS18B20_DQ2 PC1

//commands
#define DS18B20_CMD_CONVERTTEMP 0x44
#define DS18B20_CMD_RSCRATCHPAD 0xbe
#define DS18B20_CMD_WSCRATCHPAD 0x4e
#define DS18B20_CMD_CPYSCRATCHPAD 0x48
#define DS18B20_CMD_RECEEPROM 0xb8
#define DS18B20_CMD_RPWRSUPPLY 0xb4
#define DS18B20_CMD_SEARCHROM 0xf0
#define DS18B20_CMD_READROM 0x33
#define DS18B20_CMD_MATCHROM 0x55
#define DS18B20_CMD_SKIPROM 0xcc
#define DS18B20_CMD_ALARMSEARCH 0xec

//stop any interrupt on read
#define DS18B20_STOPINTERRUPTONREAD 0

#ifdef	__cplusplus
}
#endif

#endif	/* DS18B20_H */

