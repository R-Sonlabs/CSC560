#include "usart1.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <inttypes.h>
#include <stdio.h>
#include "uart1.h"
#define UART1_BAUD 19200

// UART 1: Communicate with Roomba

void usart1_init(void){
	uint16_t baud = (F_CPU / (16UL * UART1_BAUD)) - 1;
	UBRR1H = (uint8_t)(baud >> 8);
	UBRR1L = (uint8_t)baud;
	/*Enable Tx and Rx */
	UCSR1B = _BV(TXEN1) | _BV(RXEN1);	
}

//Receive a Char
int8_t usart1_getchar(void){
	loop_until_bit_is_set(UCSR1A, RXC1); // wait until data registr exists
	return UDR1;
}

//Transmit a Char
void usart1_putchar(int8_t c){
	loop_until_bit_is_set(UCSR1A, UDRE1);  // wait until data registr empty
	UDR1 = c;
}

//Transmit String
void usart1_putstring(char *str){
	while (*str == '\0') {
        uart_putchar(*str);
        str++;
    }
}

