#include <stdio.h>
#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "fnd.h"

static uint8_t digit_count;
static uint8_t fnd_data[3];
static uint8_t blink_flag;
static uint16_t blink_count;

static const uint8_t fnd_pb_array[10] = {
    _BV(FND_A) | _BV(FND_C) | _BV(FND_D) | _BV(FND_E) | _BV(FND_F),
    _BV(FND_C),
    _BV(FND_A) | _BV(FND_D) | _BV(FND_E),
    _BV(FND_A) | _BV(FND_C) | _BV(FND_D),
    _BV(FND_C) | _BV(FND_F),
    _BV(FND_A) | _BV(FND_C) | _BV(FND_D) | _BV(FND_F),
    _BV(FND_A) | _BV(FND_C) | _BV(FND_D) | _BV(FND_E) | _BV(FND_F),
    _BV(FND_A) | _BV(FND_C) | _BV(FND_F),
    _BV(FND_A) | _BV(FND_C) | _BV(FND_D) | _BV(FND_E) | _BV(FND_F),
    _BV(FND_A) | _BV(FND_C) | _BV(FND_D) | _BV(FND_F)
};

static const uint8_t fnd_pd_array[10] = {
    _BV(FND_B),
    _BV(FND_B),
    _BV(FND_B) | _BV(FND_G),
    _BV(FND_B) | _BV(FND_G),
    _BV(FND_B) | _BV(FND_G),
    _BV(FND_G),
    _BV(FND_G),
    _BV(FND_B),
    _BV(FND_B) | _BV(FND_G),
    _BV(FND_B) | _BV(FND_G)
};

static void fnd_set_display(uint8_t data, uint8_t dot)
{
    uint8_t pb, pd;
    
    if(data >= 10)
    {
        printf("FND display data ERROR : %d\r\n", data);
        return;
    }
    
    pb = PORTB;
    pd = PORTD;

    pb &= ~(_BV(FND_A) | _BV(FND_C) | _BV(FND_D) | _BV(FND_E) | _BV(FND_F));
    pd &= ~(_BV(FND_B) | _BV(FND_G) | _BV(FND_DOT));
        
    if(blink_flag == 0 || (blink_flag && blink_count < FND_BLINK_ON_COUNT))
    {
        
        pb |= fnd_pb_array[data];
        pd |= fnd_pd_array[data];

        if(dot) pd |= _BV(FND_DOT);
    }
    
    if(blink_flag)
    {
        blink_count++;
        if(blink_count >= FND_BLINK_MAX_COUNT) blink_count = 0;
    }
    
    PORTB = pb;
    PORTD = pd;
}

ISR (TIMER0_OVF_vect)
{
    TCNT0 = FND_DISPLAY_RATE;
    
    FND_DIGIT_PORT &= ~(_BV(FND_DIGIT1) | _BV(FND_DIGIT2) | _BV(FND_DIGIT3));
    
    if(digit_count == 0)
    {
        fnd_set_display(fnd_data[0], 0);
        FND_DIGIT_PORT |= _BV(FND_DIGIT1);
    }
    else if(digit_count == 1)
    {
        fnd_set_display(fnd_data[1], 1);
        FND_DIGIT_PORT |= _BV(FND_DIGIT2);
    }
    else if(digit_count == 2)
    {
        fnd_set_display(fnd_data[2], 0);
        FND_DIGIT_PORT |= _BV(FND_DIGIT3);
    }
    
    digit_count++;
    
    if(digit_count >= FND_MAX_DIGIT) digit_count = 0;
}

static void timer0_init(void)
{
	/* Timer clock = I/O clock / 1024 */
	TCCR0B = _BV(CS02) | _BV(CS00);
	/* Clear overflow flag */
	TIFR0  = _BV(TOV0);
	/* Enable Overflow Interrupt */
	TIMSK0 = _BV(TOIE0);
    
    TCNT0 = FND_DISPLAY_RATE;
}

void fnd_init(void)
{
    FND_GPIO_SET();
    timer0_init();
}

void fnd_set_data(uint16_t data)
{
    if(data > 999 || data < 0)
    {
        printf("FND data ERROR : %d, Set 0\r\n", data);
        data = 0;
    }
    
    fnd_data[0] = data / 100;
    fnd_data[1] = (data % 100) / 10;
    fnd_data[2] = data % 10;
    
    printf("FND data : %d%d%d\r\n", fnd_data[0], fnd_data[1], fnd_data[2]);
}

void fnd_blink(uint8_t blink)
{
    blink_flag = blink;
    blink_count = 0;
    printf("FND blink : %d\r\n", blink_flag);
}
