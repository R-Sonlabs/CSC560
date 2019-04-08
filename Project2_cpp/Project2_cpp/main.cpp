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

int test_periodic[] = {2,5};
void* arg1 = &test_periodic;
int test_oneS[] = {2,5};
void* arg2 = &test_oneS;

// One shot task 1 (Pin 51)
void oneShot_task_1(void* arg)
{
	//set digital pin 51

	if(arg == NULL){
		PORTB |= (1 << PB2);
		for(int x = 0; x < 1000; x++);
		PORTB &= ~(1 << PB2);
	}
	else{
		for(int i = 0; i < *(((int*)arg)); i++){
			PORTB |= (1 << PB2);
			for(int x = 0; x < 500; x++);
			PORTB &= ~(1 << PB2);
			for(int x = 0; x < 500; x++);
		}
		for(int x = 0; x < 1000; x++);
		for(int i = 0; i < *(((int*)arg+1)); i++){
			PORTB |= (1 << PB2);
			for(int x = 0; x < 500; x++);
			PORTB &= ~(1 << PB2);
			for(int x = 0; x < 500; x++);
		}
	}
}

// Periodic Task 1
void periodic_task_1(void* arg)
{
	//set digital pin 52 high
	if (arg == NULL)
	{
		PORTB |= (1 << PB1);
		for(int x = 0; x < 3000; x++);
		PORTB &= ~(1 << PB1);
	}
	else{
		for(int i = 0; i < *(((int*)arg)); i++){
			PORTB |= (1 << PB1);
			for(int x = 0; x < 500; x++);
			PORTB &= ~(1 << PB1);
			for(int x = 0; x < 500; x++);
		}
		for(int x = 0; x < 1000; x++);
		for(int i = 0; i < *(((int*)arg+1)); i++){
			PORTB |= (1 << PB1);
			for(int x = 0; x < 500; x++);
			PORTB &= ~(1 << PB1);
			for(int x = 0; x < 500; x++);
		}
	}
}

// Periodic Task 2
void periodic_task_2(void* arg)
{
	//set digital pin 53 high
	if (arg == NULL)
	{
		PORTB |= (1 << PB0);
		for(int x = 0; x < 3000; x++);
		PORTB &= ~(1 << PB0);
	}
	else{
		for(int i = 0; i < *(((int*)arg)); i++){
			PORTB |= (1 << PB0);
			for(int x = 0; x < 500; x++);
			PORTB &= ~(1 << PB0);
			for(int x = 0; x < 500; x++);
		}
		for(int x = 0; x < 1000; x++);
		for(int i = 0; i < *(((int*)arg+1)); i++){
			PORTB |= (1 << PB0);
			for(int x = 0; x < 500; x++);
			PORTB &= ~(1 << PB0);
			for(int x = 0; x < 500; x++);
		}
	}
}

// Periodic Task 3
void periodic_task_3(void* arg)
{
	//set digital pin 51 high
	PORTB |= (1 << PB2);
	for(int x = 0; x < 1000; x++);
	PORTB &= ~(1 << PB2);
	Scheduler_StartTask(10, 30, periodic_task_1, NULL);
}

// idle task (PC7)
void idle(uint32_t idle_period)
{
	PORTB |= (1 << PB3);
	//for(int x = 0; x < 1000; x++);
	Scheduler_Dispatch_Oneshot(idle_period);
	PORTB &= ~(1 << PB3);
}

void success(){
	Scheduler_StartTask(0, 20, periodic_task_1, NULL);
	Scheduler_StartTask(15, 30, periodic_task_2, NULL);
	Scheduler_StartTask(20, 40, periodic_task_2, arg1);
	Scheduler_StartTask_Oneshot(15, 15, oneShot_task_1, NULL,1);
	Scheduler_StartTask_Oneshot(15, 2, oneShot_task_1, NULL,0);
}
void taskCollide(){
	Scheduler_StartTask(0, 20, periodic_task_1, NULL);
	Scheduler_StartTask(0, 20, periodic_task_2, NULL);
}
void time_con(){
	Scheduler_StartTask(20, 40, periodic_task_2, NULL);
	Scheduler_StartTask_Oneshot(40, 30, oneShot_task_1, NULL,0);
}
void too_many_OTask(){
	Scheduler_StartTask(0, 10, periodic_task_1, NULL);
	Scheduler_StartTask(15, 10, periodic_task_2, NULL);
	Scheduler_StartTask_Oneshot(10, 10, oneShot_task_1, NULL,0);
	Scheduler_StartTask_Oneshot(30, 5, oneShot_task_1, arg2,0);
}

void isrTooLong(){
	Scheduler_StartTask(5, 20, periodic_task_1, NULL);
	Scheduler_StartTask(15, 30, periodic_task_2, NULL);
	Scheduler_StartTask_Oneshot(30, 5, oneShot_task_1, arg2,0);
	Scheduler_StartTask_Oneshot(700, 10, oneShot_task_1, NULL,0);
}
void setup()
{
	//PB0: 53
	//PB1: digital pin 52
	//PB2: digital pin 51
	//PB3: digital pin 50
	Scheduler_Init();
	//Scheduler_StartTask(0, 20, periodic_task_1, NULL);
	//Scheduler_StartTask(15, 30, periodic_task_2, NULL);
	//-- Success --
	success();

	//-- Time conflict between oneshot tasks and periodic task --
	//time_con();
	
	//-- Two periodic tasks collide repeatedly --
	//taskCollide();

	//-- Too much oneshot tasks, miss deadline --
	//too_many_OTask();

	//-- Interrupt handler executing too long, miss deadline --
	//isrTooLong();
}



int main(void)
{
	DDRB = 0xFF;
	DDRA |= (1<<PA0);
	PORTA &= ~(1 << PA0);
    setup();
    for (;;)
    {
	    uint32_t idle_period = Scheduler_Dispatch();
	    if (idle_period)
	    {
		    idle(idle_period);
	    }
    }
    for (;;);
    return 0;
}

