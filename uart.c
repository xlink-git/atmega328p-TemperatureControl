#include <stdio.h>
#include "uart.h"
#include <avr/io.h>

#define UART_BAUDRATE   38400

static int _uart_tx(char ch, struct __file *not_used)
{
	if (ch == '\n')
        _uart_tx('\r', 0);

	while (!(UCSR0A & _BV(UDRE0)));
    
	UDR0 = ch;
    return 1;
} 

static int _uart_rx(struct __file *not_used)
{
	uint8_t data;

	while (!(UCSR0A & _BV(RXC0)));
    
	data = UDR0;
	_uart_tx(data, 0);
    
	return data;
}
static FILE OUTPUT = FDEV_SETUP_STREAM(_uart_tx, NULL, _FDEV_SETUP_WRITE);
static FILE INTPUT = FDEV_SETUP_STREAM(NULL, _uart_rx, _FDEV_SETUP_READ);

void uart_init(void)
{
	uint16_t baudrate = F_CPU/UART_BAUDRATE/16 - 1;

	UCSR0B = 0x00;
	UCSR0A = 0x00;
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01); // N81
    
    UBRR0 = baudrate;
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);   // Enable Tx and Rx
	
	stdout =  &OUTPUT;
	stdin  =   &INTPUT;
	
}

void uart_tx(uint8_t ch)
{
	while (!(UCSR0A & _BV(UDRE0)));
    
	UDR0 = ch;
}

int8_t uart_rx(uint8_t *data)
{
	if(!(UCSR0A & _BV(RXC0))) return -1;
    
	*data = UDR0;
    
	return 0;
}
