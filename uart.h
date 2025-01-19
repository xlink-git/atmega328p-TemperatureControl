#ifndef UART_LIBRARY_H
#define UART_LIBRARY_H

void uart_init(void);
void uart_tx(uint8_t ch);
int8_t uart_rx(uint8_t *data);

#endif
