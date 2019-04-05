/*
 Servo.cpp - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
 Copyright (c) 2009 Michael Margolis.  All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if defined(ARDUINO_ARCH_AVR)

#include <avr/interrupt.h>
#include <Arduino.h>
#include <avr/io.h>
#include "Server.h"


static servo_t servos[MAX_SERVOS];
uint8_t ServoCount = 0;

/* Replace with your library code */
void timer_init(){
	/*---------- Motor Initialization ----------
	* xPin 7 (OC3C/INT5) = digital pin 3 = PE5
	* yPin 5 (OC3A/AIN1) = digital pin 5 = PE3
	* Output Compare Register 1 B and 1 C
	*/
	// Set PORTE Pin 5 (Digital Pin 3) as output
	DDRE |= (1 << DDE5);
	// Set PORTE Pin 3 (Digital Pin 5) as output
	DDRE |= (1 << DDE3);

	// Set PWM
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	TCCR1B |= (0 << WGM12);
	// Clear OC1C on Compare Match when up-counting. Set OC1C on Compare Match when down-counting
	TCCR1A |= (1 << COM1C1) | (0 << COM1C0);
	// Clear OC1B on Compare Match when up-counting. Set OC1B on Compare Match when down-counting
	TCCR1A |= (1 << COM1B1) | (0 << COM1B0);
	// clk(I/O)/(256)
	TCCR1B |= (1 << CS12) | (0 << CS11) | (0 << CS10);
	// Initialize base motor (Digital Pin 3) 49/1024 * (8/256) = 1500 Lower bound: ?; Upper bound: ?; Middle: ?
	//OCR1C = 49;
	// Initialize arm motor (Digital Pin 5) Lower bound: ?; Upper bound: ?; Middle: ?
	//OCR1B = 49;
}


Servo::Servo(){
	if(ServoCount < MAX_SERVOS){
		this->servoIndex = ServoCount++;                    // assign a servo index to this instance
    	servos[this->servoIndex].ticks = DEFAULT_PULSE_WIDTH;   // store default values  
	}
}

uint8_t Servo::attach(int pin, int min, int max){
	if(this->servoIndex < MAX_SERVOS) {
		pinMode(pin, OUTPUT);
		this->min  = (MIN_PULSE_WIDTH - min)/4; //resolution of min/max is 4 uS
    	this->max  = (MAX_PULSE_WIDTH - max)/4;
    	timer_init();
	}
	return this->servoIndex;
}

void Servo::write(int val){
	Serial.println("This is servo write function");
	byte channel = this->servoIndex;
	if( (channel >= 0) && (channel < MAX_SERVOS)){  
		if( val < SERVO_MIN()){
			val = SERVO_MIN();
		}else if( val > SERVO_MAX()){
			val = SERVO_MAX(); 
		}
		val = (val-5);
		val = usToTicks(val);  // convert to ticks after compensating for interrupt overhead
		Serial.println("S: ");
		Serial.print(val);
	    servos[channel].ticks = val;
	}
}

int Servo::read(){
	unsigned int pulsewidth;
  	if (this->servoIndex < MAX_SERVOS)
    	pulsewidth = ticksToUs(servos[this->servoIndex].ticks) + 5;
  	else
    	pulsewidth  = 0;
  	return pulsewidth;
}

#endif // ARDUINO_ARCH_AVR

