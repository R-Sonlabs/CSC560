/*
 * scheduler.cpp
 *
 * Created: 3/25/2019 11:35:29 PM
 *  Author: grace
 * scheduler.cpp
 *
 *  Created on: 17-Feb-2011
 *      Author: nrqm
 *      Based on code by Jacob Schwartz
 */
#include "scheduler.h"
#include <math.h>
#include <avr/interrupt.h>
//#define NULL 0
typedef struct
{
	int32_t period;
	int32_t remaining_time;
	uint8_t is_running;
	task_cb callback;
	void* argument;         //generic argument
} task_t;

typedef struct
{
	int32_t remaining_time;
	uint8_t is_running;
	task_cb callback;
	int priority;
	void* argument;      //generic argument
	int32_t run_time;
} oneshot_t;

task_t tasks[MAXTASKS];
oneshot_t oneS_tasks[MAXTASKS];
uint32_t last_runtime;
uint32_t idle_start;
uint32_t last_oneshot_time;
int32_t remaining_idle_time;

#define Disable_Interrupt()		asm volatile ("cli"::)
#define Enable_Interrupt()		asm volatile ("sei"::)
volatile unsigned long ms = 0;

void Scheduler_Init()
{
  last_runtime = ms;
  last_oneshot_time = ms;  
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
  Enable_Interrupt();
 
}

//periodic tasks
void Scheduler_StartTask(int16_t delay, int16_t period, task_cb task, void* argument)
{
  static uint8_t id = 0;
  if (id < MAXTASKS)
  {
    tasks[id].remaining_time = delay;
    tasks[id].period = period;
    tasks[id].is_running = 1;
    tasks[id].callback = task;
	tasks[id].argument = argument;
    id++;
  }
}
 
uint32_t Scheduler_Dispatch()
{
  uint8_t i;
  uint32_t now = ms;
  uint32_t elapsed = now - last_runtime;
  last_runtime = now;
  task_cb t = NULL;
  uint32_t idle_time = 0xFFFFFFFF;
  int index = -1;
  void* argument = NULL;
  // update each task's remaining time, and identify the first ready task (if there is one).
  for (i = 0; i < MAXTASKS; i++)
  {
    if (tasks[i].is_running)
    {
      // update the task's remaining time
      tasks[i].remaining_time -= elapsed;
      if (tasks[i].remaining_time <= 0)
      {
        if (t == NULL)
        {
          // if this task is ready to run, and we haven't already selected a task to run,
          // select this one.
		  index = i;
          t = tasks[i].callback;
          tasks[i].remaining_time += tasks[i].period;
		  argument = tasks[i].argument;
        }
        idle_time = 0;
      }
      else
      {
        idle_time = fmin((uint32_t)tasks[i].remaining_time, idle_time);
      }
    }
  }
  if (t != NULL)
  {
    // If a task was selected to run, call its function.
	Disable_Interrupt();
    t(argument);
	Enable_Interrupt();
  }
  return idle_time;
}

//---Oneshot tasks--- 
void Scheduler_StartTask_Oneshot(int32_t remaining_time, int32_t run_time, task_cb task, void* argument, int priority)
{
	static uint8_t id = 0;
	while (id < MAXTASKS && !oneS_tasks[id].is_running)
	{
		oneS_tasks[id].remaining_time = remaining_time;
		oneS_tasks[id].run_time = run_time;
		oneS_tasks[id].is_running = 1;
		oneS_tasks[id].callback = task;
		oneS_tasks[id].argument = argument;
		oneS_tasks[id].priority = priority;
		id++;
	}
}

void Scheduler_Dispatch_Oneshot(uint32_t idle_time)
{	
	uint8_t i;
	int index = -1;
	uint32_t now = ms;
	uint32_t elapsed = now - last_oneshot_time;
	last_oneshot_time = now;
	task_cb t = NULL;
	void* argument = NULL;
	
	for (i = 0; i < MAXTASKS; i++)
	{
		if (oneS_tasks[i].is_running)
		{
			// update the task's remaining time
			oneS_tasks[i].remaining_time -= elapsed;
			if (oneS_tasks[i].remaining_time <= 0 && idle_time >= oneS_tasks[i].run_time && t==NULL)
			{
				index = i;
				t = oneS_tasks[i].callback;
				argument = oneS_tasks[i].argument;
				//oneS_tasks[i].is_running = 0;
			}
		}
	}
	if (t != NULL)
	{
		// If a task was selected to run, call its function.
		// --TODO: check oneshot tasks ---
		Disable_Interrupt();
		t(argument);
		Enable_Interrupt();
		//tasks[index].remaining_time = tasks[index].period;
	}	
}

ISR(TIMER1_COMPA_vect) {
	Disable_Interrupt();
	PORTA |= (1<<PA0);
	ms ++;
	PORTA &= ~(1 << PA0);
	Enable_Interrupt();
}