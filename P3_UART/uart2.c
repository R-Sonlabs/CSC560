#include "uart2.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <inttypes.h>
#include <stdio.h>
#define UART2_BAUD 19200

//UART 2: Bluetooth

void usart2_init(void){
	uint16_t baud = (F_CPU / (16UL * UART2_BAUD)) - 1;
	UBRR2H = (uint8_t)(baud >> 8);
	UBRR2L = (uint8_t)baud;
	/*Enable Tx and Rx */
	UCSR2B = _BV(TXEN2) | _BV(RXEN2);
}


int8_t usart2_getchar(void){
	loop_until_bit_is_set(UCSR2A, RXC2); // wait until data registr exists
	return UDR2;
}


void usart2_putchar(int8_t c){
	loop_until_bit_is_set(UCSR2A, UDRE2);  // wait until data registr empty
	UDR2 = c;
}
