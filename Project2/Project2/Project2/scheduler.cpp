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
//#include "arduino/Arduino.h"
#include <avr/interrupt.h>
//#define NULL 0

volatile unsigned long ms;
//volatile unsigned long system_timer = 0;
typedef struct
{
  int32_t period;
  int32_t remaining_time;
  uint8_t is_running;
  task_cb callback;
} task_t;

typedef struct
{
	int32_t period;
	int32_t remaining_time;
	uint8_t is_running;
	task_cb callback;
	int priority;
	int32_t run_time;
	
} oneshot_t;

task_t tasks[MAXTASKS];
 
uint32_t last_runtime;

void Scheduler_Init()
{
  last_runtime = ms;
}
 
void Scheduler_StartTask(int16_t delay, int16_t period, task_cb task)
{
  static uint8_t id = 0;
  if (id < MAXTASKS)
  {
    tasks[id].remaining_time = delay;
    tasks[id].period = period;
    tasks[id].is_running = 1;
    tasks[id].callback = task;
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
          t = tasks[i].callback;
          tasks[i].remaining_time += tasks[i].period;
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
    t();
  }
  return idle_time;
}

ISR(TIMER1_COMPA_vect) {
	ms++;
}


//
//void Timer0_Init(void)
//{
	//TCCR0A = 0x07;
	//TCNT0 = 0xF0;
	//TIMSK0 = 0x01;
//}