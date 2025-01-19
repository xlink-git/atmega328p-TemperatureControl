# atmega328p-TemperatureControl
ATmega328P 를 사용한 온도조절기

- Author : Kwon Taeyoung (xlink69@gmail.com)
- IDE : MPLAB X IDE v6.20
- Compiler : AVR-GCC v9.4.0
- ATmega DFP : 3.1.246
- Peprpherals : UART, ADC, GPIO, PWM, Timer, NTC, DS18B20, Internal EEPROM

# I/O connections
- NTC : ADC6
- Heater PWM : PB1
- FND Digit 1 : PD3
- FND Digit 2 : PD4
- FND Digit 3 : PD5
- FND a : PB0
- FND b : PD2
- FND c : PB2
- FND d : PB3
- FND e : PB4
- FND f : PB5
- FND g : PD6
- FND dot : PD7
- Set SW : PC3
- Up SW : PC5
- Down SW : PC4
- UART Tx : PD1
- UART Rx : PD0
