/*
	Roomba Controller
	CSC560 - Project 3
	UVIC, Spring 2019
	
	Matt Richardson & Grace Liu
	Team 9
*/


#include <Arduino.h>
typedef void (*FunctionPointer)();
void queueMaker(FunctionPointer func, int offset);
int test();
void idler();
void fireLaser();
void getVelocity();
void getRadius();
void getXval();
void getYval();
void btTransmit();
void timerMaker();
void button1Read ();
void button2Read ();
void button3Read ();
ISR(TIMER0_COMPA_vect );
ISR(TIMER5_OVF_vect );

#define period 10000  //GRACE: This is how frequently the timer fires off the next task
#define testPin 52 //GRACE: just use this for when you need to test if a task is firing
#define maxFuncs 9 //GRACE: increment this for every task you add
#define xPin A0
#define yPin A1
#define vPin A2
#define rPin A3
#define zPin 7
#define button1 12
#define button2 11
#define button3 10      

FunctionPointer funcs[maxFuncs];

int funcCounter;
int velocity;
int radius;
int xPwm;
int yPwm;
int xVal;
int yVal;
int vVal;
int rVal;
bool button1Val;
bool button2Val;
bool button3Val;
bool trigger;
bool idleFlag = true;

void setup() {
	pinMode(testPin, OUTPUT);
	pinMode(zPin, INPUT_PULLUP);
	pinMode(button1, INPUT_PULLUP);
	pinMode(button2, INPUT_PULLUP);
	pinMode(button3, INPUT_PULLUP);
	Serial.begin(19200);
	Serial1.begin(19200);
	
	//FunctionPointer testFunc = &test;  //GRACE: Declare each task this way
	//queueMaker(testFunc, 0);  //GRACE: Put tasks in queue like this.  The int is for it's order.

	//task queue:
	FunctionPointer triggerButton = &fireLaser;
	queueMaker(triggerButton, 0);
	FunctionPointer getV = &getVelocity;
	queueMaker(getV, 1);
	FunctionPointer getR = &getRadius;
	queueMaker(getR, 2);
	FunctionPointer getX = &getXval;
	queueMaker(getX, 3);
	FunctionPointer getY = &getYval;
	queueMaker(getY, 4);
	FunctionPointer sendData = &btTransmit;
	queueMaker(sendData, 5);
	FunctionPointer getButton1 = &button1Read;
	queueMaker(getButton1, 6);
	FunctionPointer getButton2 = &button2Read;
	queueMaker(getButton2, 7);
	FunctionPointer getButton3 = &button3Read;
	queueMaker(getButton3, 8);
	
	timerMaker();
	idler();
}

void queueMaker(FunctionPointer func, int offset){
	funcs[offset] = func;
}

int test(){  //Just a testing function to call for debbugging
	digitalWrite(testPin, digitalRead(testPin) ^ 1);
}

void idler(){
	
	while (1){
		digitalWrite(53, digitalRead(53) ^ 1);
	}

}

void fireLaser(){
	trigger = !digitalRead(zPin);
}

void getVelocity(){
	vVal = analogRead(vPin);
    if (vVal >= 1000){
	    velocity = 200;
    }
    if (vVal > 750 && vVal < 1000){
	    velocity = 100;
    }
    if (vVal < 350 && vVal > 10){
	    velocity = -100;
    }
    if (vVal <= 10){
	    velocity = -200;
    }
    if (vVal >= 350 && vVal <= 750){
	    velocity = 0;
    }
	
}

void getRadius(){
	rVal = analogRead(rPin);
    if (rVal >= 900){
	    radius = -200;
    }
    if (rVal > 600 && rVal < 900){
	    radius = -100;
    }
    if (rVal < 300 && rVal > 10){
	    radius = 100;
    }
    if (rVal <= 10){
	    radius = 200;
    }
    if (rVal >= 300 && rVal <= 600){
	    radius = 0;
    }
}

void getXval(){
	xVal = analogRead(xPin);
	xPwm = constrain(xPwm, 15, 160);
	if (xVal >= 1000){
	  xPwm = xPwm + 2;
	}
	if (xVal > 600 && xVal < 1000){
		xPwm = xPwm + 1;
	}
	if (xVal < 350 && xVal > 10){
		xPwm = xPwm - 1;
	}
	if (xVal <= 10){
		xPwm = xPwm - 2;
	}
	if (xVal >= 350 && xVal <= 600){
		xPwm = xPwm;
	}
}

void getYval(){
	yVal = analogRead(yPin);
	yPwm = constrain(yPwm, 15, 160);
	int rawy = analogRead(yPin);
	if (yVal >= 1000){
		yPwm = yPwm + 2;
	}
	if (yVal > 600 && yVal < 1000){
		yPwm = yPwm + 1;
	}
	if (yVal < 350 && yVal > 10){
		yPwm = yPwm - 1;
	}
	if (yVal <= 10){
		yPwm = yPwm - 2;
	}
	if (yVal >= 350 && yVal <= 600){
		yPwm = yPwm;
	}
}

void button1Read(){
	button1Val = !digitalRead(button1);
}

void button2Read(){
	button2Val = !digitalRead(button2);
}

void button3Read(){
	button3Val = !digitalRead(button3);
}

void btTransmit(){
	Serial1.print("<");
	Serial1.print(xPwm);
	Serial1.print(",");
	Serial1.print(yPwm);
	Serial1.print(",");
	Serial1.print(trigger);
	Serial1.print(",");
	Serial1.print(velocity);
	Serial1.print(",");
	Serial1.print(radius);
	Serial1.print(",");
	Serial1.print(button1Val);
	Serial1.print(",");
	Serial1.print(button2Val);
	Serial1.print(",");
	Serial1.print(button3Val);
	Serial1.print(">");

	Serial.print("<");
	Serial.print(xPwm);
	Serial.print(",");
	Serial.print(yPwm);
	Serial.print(",");
	Serial.print(trigger);
	Serial.print(",");
	Serial.print(velocity);
	Serial.print(",");
	Serial.print(radius);
	Serial.print(",");
	Serial.print(button1Val);
	Serial.print(",");
	Serial.print(button2Val);
	Serial.print(",");
	Serial.print(button3Val);
	Serial.println(">");
}

void timerMaker() {
	TCCR0A = 0;
	TCCR0B = 0;
	TCNT0  = 0;
	OCR0A = period;
	TCCR0B |= (1 << WGM02);
	TCCR0B |= (1 << CS02) | (1 << CS00);
	TIMSK0 |= (1 << OCIE0A);
	
	TCCR5A = 0;
	TCCR5B = 0;
	TCNT5  = 0;
	TCCR5B |= (1 << CS52);
	TIMSK5 |= (1 << TOIE5);
}


ISR(TIMER0_COMPA_vect)          // Fires off a task each time our period comes up.  I need to fix this crap

{
	(*funcs[funcCounter])();
	funcCounter++;
	if (funcCounter == maxFuncs)
	funcCounter = 0;
}

ISR(TIMER5_OVF_vect)        // Not really used in this implementation at the moment.  Left in in case I need it.
{
	TCNT5 = 34286;
}

int main(void) {
	init();
	setup();
	while (1)
	{
	}
}
