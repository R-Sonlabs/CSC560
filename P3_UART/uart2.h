#ifndef ___UART2_H_INCLUDED_
#define ___UART2_H_INCLUDED_


#define F_CPU 16000000UL
#include <inttypes.h>


void usart2_init(void);
int8_t usart2_getchar(void);
void usart2_putchar(int8_t c);

