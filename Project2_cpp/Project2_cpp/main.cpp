/*
 * main.cpp
 *
 * Created: 3/26/2019 12:23:47 AM
 * Author : grace
 * Example program for time-triggered scheduling on Arduino.
 *
 * This program pulses one pin every 500 ms and another pin every 300 ms
 *
 * There are two important parts in this file:
 *    - at the end of the setup function I call Scheduler_Init and Scheduler_StartTask.  The latter is called once for
 *      each task to be started (note that technically they're not really tasks because they share the stack and don't
 *      have separate contexts, but I call them tasks anyway because whatever).
 *    - in the loop function, I call the scheduler dispatch function, which checks to see if a task needs to run.  If
 *      there is a task to run, then it its callback function defined in the StartTask function is called.  Otherwise,
 *      the dispatch function returns the amount of time (in ms) before the next task will need to run.  The idle task
 *      can then do something useful during that idle period (in this case, it just hangs).
 *
 * To use the scheduler, you just need to define some task functions and call Scheduler_StartTask in the setup routine
 * for each of them.  Keep the loop function below the same.  The scheduler will call the tasks you've created, in
 * accordance with the creation parameters.
 *
 * See scheduler.h for more documentation on the scheduler functions.  It's worth your time to look over scheduler.cpp
 * too, it's not difficult to understand what's going on here.
 */ 

#include "scheduler.h"
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/sfr_defs.h>
#define TICKS_PER_SECOND 16000000L
volatile unsigned long system_timer = 0;

#define Disable_Interrupt()		asm volatile ("cli"::)
#define Enable_Interrupt()		asm volatile ("sei"::)
LinkedList <task_arg> arg1;
LinkedList <task_arg> arg2;

// Get timer value
unsigned long get_system_timer( void )
{
	unsigned char sreg;
	unsigned long i;
	/* Save global interrupt flag */
	sreg = SREG;
	/* Disable interrupts */
	cli();
	/* Read TCNT1 into i */
	i = system_timer;
	/* Restore global interrupt flag */
	SREG = sreg;
	return i;
}

//my delay function
void delay(uint32_t ms)
{
	unsigned long current_time;
	unsigned long previous_time = get_system_timer();
	
	for(;;){
		current_time = get_system_timer();
		if(current_time - previous_time >= ms){
			break;
		}
		sleep_enable(); // put the processor into sleep mode while waiting for the timer event
	}
}

// One shot task 1 (Pin 5)
void pulse_pin5_task(LinkedList<task_arg> &arg1)
{
	//set digital pin 5
	PORTB |= (1 << PB7);
	delay(10);
	PORTB &= ~(1 << PB7);
}

// One shot task 2 (Pin 6)
void pulse_pin6_task(LinkedList<task_arg> &arg1)
{
	//set digital pin 6
	PORTD |= (1 << PD6);
	Scheduler_StartTask_Oneshot(pulse_pin5_task, arg2, 0, 10, 70);
	PORTD &= ~(1 << PD6);
}

// task 2 (Pin 7)
void pulse_pin7_task(LinkedList<task_arg> &arg1)
{
	//set digital pin 7
	PORTD |= (1 << PD7);
	delay(10);
	PORTD &= ~(1 << PD7);
}

// Periodic Task 1
void periodic_task_1(LinkedList<task_arg> &arg1)
{
	//set digital pin 3 high
	PORTD |= (1 << PD3);
	Scheduler_StartTask_Oneshot(pulse_pin5_task, arg2, 0, 10, 70);
	Scheduler_StartTask_Oneshot(pulse_pin6_task, arg2, 0, 20, 70);
	Scheduler_StartTask_Oneshot(pulse_pin5_task, arg2, 1, 10, 90);
	PORTD &= ~(1 << PD3);
}

// Periodic Task 2
void periodic_task_2(LinkedList<task_arg> &arg1)
{
	//set digital pin 4 high, and then 4 low
	PORTD |= (1 << PD4);
	Scheduler_StartTask_Oneshot(pulse_pin5_task, arg2, 1, 10, 70);
	Scheduler_StartTask_Oneshot(pulse_pin6_task, arg2, 0, 10, 30);
	PORTD &= ~(1 << PD4);
}

// Periodic Task 3
void periodic_task_3(LinkedList<task_arg> &arg1)
{
	//set digital pin 3 high
	PORTD |= (1 << PD3);
	Scheduler_StartTask_Oneshot(pulse_pin5_task, arg2, 0, 10, 70);
	Scheduler_StartTask_Oneshot(pulse_pin6_task, arg2, 0, 20, 70);
	Scheduler_StartTask_Oneshot(pulse_pin6_task, arg2, 1, 10, 90);
	Scheduler_StartTask_Oneshot(pulse_pin6_task, arg2, 1, 15, 70);
	Scheduler_StartTask_Oneshot(pulse_pin6_task, arg2, 1, 15, 90);
	PORTD &= ~(1 << PD3);
}

// Periodic Task 4
void periodic_task_4(LinkedList<task_arg> &arg1)
{
	//set digital pin 3 high
	PORTD |= (1 << PD3);
	Scheduler_StartTask_Oneshot(pulse_pin5_task, arg2, 0, 10, 70);
	Scheduler_StartTask_Oneshot(pulse_pin6_task, arg2, 0, 20, 70);
	Scheduler_StartTask_Oneshot(pulse_pin6_task, arg2, 1, 60, 90);
	PORTD &= ~(1 << PD3);
}


// idle task (Pin 2)
void idle(uint32_t idle_period)
{
	// this function can perform some low-priority task while the scheduler has nothing to run.
	// It should return before the idle period (measured in ms) has expired.  For example, it
	// could sleep or respond to I/O.
	
	// example idle function that just pulses a pin.
	PORTH |= (1 << PH6);
	delay(idle_period);
	Scheduler_Dispatch_Oneshot();
	PORTH &= ~(1 << PH6);
}

void setup()
{
	DDRD |= (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
	DDRB |= (1 << PB7);
	
	/*-----Apply Timer Interrupt------*/
	Disable_Interrupt();
	TCCR1A = 0; // set entire TCCR1A register to 0
	TCCR1B = 0; // same for TCCR1B
	TCNT1  = 0; // initialize counter value to 0
	// set compare match register for 100 Hz increments
	OCR1A = 16000000 / (8 * 1000);
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS12, CS11 and CS10 bits for 8 prescaler
	TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);
	Scheduler_Init();
	Enable_Interrupt();
	/* Add Test Case Here!
	 * check time conflict between Non-time Critical task and Time Triggered task
	 * two time based tasks collide repeatedly
	 * too many one-shot tasks cause the time-based tasks to miss their deadlines
	*/
	//Success
	//Scheduler_StartTask(0, 300, periodic_task_1, arg1);
	Scheduler_StartTask(50, 300, pulse_pin5_task, arg1);
	
	//Time conflict between oneshot tasks and periodic task
	//Scheduler_StartTask(0, 300, periodic_task_1, arg1);
	//Scheduler_StartTask(0, 300, pulse_pin7_task, arg1);
	
	// Two periodic tasks collide repeatedly
	//Scheduler_StartTask(0, 200, pulse_pin6_task, arg1);
	//Scheduler_StartTask(0, 300, pulse_pin7_task, arg1);
	
	//Too much oneshot tasks, miss deadline
	//Scheduler_StartTask(0, 200, periodic_task_3, arg1);
	//Scheduler_StartTask(50, 300, pulse_pin7_task, arg1);
	
	//Interrupt handler executing too long, miss deadline
	//Scheduler_StartTask(0, 200, periodic_task_4, arg1);
	//Scheduler_StartTask(50, 300, pulse_pin7_task, arg1);
}

void loop()
{
	uint32_t idle_period = Scheduler_Dispatch();
	if (idle_period)
	{
		idle(idle_period);
	}
}


int main(void)
{
    setup();
    for (;;)
    {
	    loop();
    }
    for (;;);
    return 0;
}

