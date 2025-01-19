/* 
 * File:   fnd.h
 * Author: xlink
 *
 * Created on March 1, 2024, 2:30 AM
 */

#ifndef FND_H
#define	FND_H

#define FND_MAX_DIGIT       5       // ?? ??? 2??? ???? ?? ?? ??

#define FND_DISPLAY_RATE    (256-44)    // about 180 Hz : 16M/1024/87
#define FND_BLINK_ON_COUNT  (75)
#define FND_BLINK_MAX_COUNT (90)

//#define FND_DISPLAY_RATE    (256-65)    // about 240 Hz : 16M/1024/65

#define FND_DIGIT_PORT  PORTD

#define FND_DIGIT1  PD3
#define FND_DIGIT2  PD4
#define FND_DIGIT3  PD5

#define FND_A       PB0
#define FND_B       PD2
#define FND_C       PB2
#define FND_D       PB3
#define FND_E       PB4
#define FND_F       PB5
#define FND_G       PD6
#define FND_DOT     PD7

#define FND_GPIO_SET()  DDRB |= (_BV(FND_A) | _BV(FND_C) | _BV(FND_D) | _BV(FND_E) | _BV(FND_F));   \
                        DDRD |= (_BV(FND_DIGIT1) | _BV(FND_DIGIT2) | _BV(FND_DIGIT3) | _BV(FND_B) | _BV(FND_G) | _BV(FND_DOT))

#ifdef	__cplusplus
extern "C" {
#endif

void fnd_init(void);
void fnd_set_data(uint16_t data);
void fnd_blink(uint8_t blink);

#ifdef	__cplusplus
}
#endif

#endif	/* FND_H */

