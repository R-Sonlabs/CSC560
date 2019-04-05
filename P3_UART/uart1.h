#ifndef ___USART1_H_INCLUDED_
#define ___USART1_H_INCLUDED_


#define F_CPU 16000000UL
#include <inttypes.h>


void usart1_init(int use_interrupt);
int8_t usart1_getchar(void);
void usart1_putchar(int8_t c);
char usart1_getstring(void);
void usart1_putstring(char *str);
