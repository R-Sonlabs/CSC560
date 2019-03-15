/*
 * Project2.c
 *
 * Created: 3/12/2019 1:33:53 AM
 * Author : grace
 * main.cpp
 *
 *  Created on: 18-Feb-2011
 *      Author: nrqm
 *
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
 
//#include "arduino/Arduino.h"
#include "scheduler.h"
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/sfr_defs.h>

//Port D (digital pins 0 to 7)
//uint8_t pulse1_pin = 3;
//uint8_t pulse2_pin = 4;
//uint8_t idle_pin = 7;

volatile unsigned long system_timer = 0;

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

// task function for PulsePin task
void ping()
{
  //set digital pin 3, 4 to high
  PORTD |= (1 << PD3);
  PORTD |= (1 << PD4);

}
 
// task function for PulsePin task
void pong()
{
  //set digital pin 3, 4 to low
  PORTD &= ~(1 << PD3);
  PORTD &= ~(1 << PD4);
}
 
// idle task
void idle(uint32_t idle_period)
{
  // this function can perform some low-priority task while the scheduler has nothing to run.
  // It should return before the idle period (measured in ms) has expired.  For example, it
  // could sleep or respond to I/O.
 
  // example idle function that just pulses a pin.
  PORTD |= (1 << PD7);
  delay(idle_period);
  PORTD &= ~(1 << PD7);
}
 
void setup()
{
  // pinMode(pulse1_pin, OUTPUT);
  // pinMode(pulse2_pin, OUTPUT);
  // pinMode(idle_pin, OUTPUT);
  DDRD |= (1 << PD3) | (1 << PD4) | (1 << PD7);
  Scheduler_Init();
 
  // Start task arguments are:
  //    start offset in ms, period in ms, function callback
 
  Scheduler_StartTask(0, 500, ping);
  Scheduler_StartTask(0, 300, pong);
}
 
void loop()
{
  uint32_t idle_period = Scheduler_Dispatch();
  if (idle_period)
  {
    idle(idle_period);
  }
}
 
int main()
{

  //init();
  setup();
 
  for (;;)
  {
    loop();
  }
  for (;;);
  return 0;
}

