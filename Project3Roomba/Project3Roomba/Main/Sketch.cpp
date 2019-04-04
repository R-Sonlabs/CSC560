#include <Arduino.h>
#include <Servo.h>



typedef void (*FunctionPointer)();

int queueMaker(FunctionPointer func, int offset);
int test();
void idler();
void timerMaker();
void btListen();
void parseData();
void showParsedData();
void driving();
void servoing();
void river();
void wakeUp(void );
void stopDrive(void );
void laser();
void champions();
void lightSensor();
ISR(TIMER0_COMPA_vect );
ISR(TIMER5_OVF_vect );


#define period 100  //GRACE: This is how frequently the timer fires off the next task
#define testPin 52 //GRACE: just use this for when you need to test if a task is firing
#define maxFuncs 6 //GRACE: increment this for every task you add
#define ddPin 12
#define laserPin 9
#define xPin 7
#define yPin 5
#define photoSensor A8
#define photoThreshold 500
#define clamp(value, min, max) (value < min ? min : value > max ? max : value)




FunctionPointer funcs[maxFuncs];

int funcCounter;
int velocity;
int oldVel;
int radius;
int oldRad;
int xVal;
int oldX;
int yVal;
int oldY;
int shot;
bool shotFlag;
bool trigger;
bool oldTrigger;
char inData[80];
byte index = 0;
bool idleFlag = true;
bool riverFlag = false;
int laserTimer = 0;
int driveTimer = 0;
int photoVal;


const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];
boolean newData = false;

Servo servoX, servoY;

void setup() {
	pinMode(testPin, OUTPUT);
	pinMode(ddPin, OUTPUT);
	pinMode(53, OUTPUT);
	pinMode(51, OUTPUT);
	pinMode(49, OUTPUT);
	pinMode(47, OUTPUT);
	pinMode(laserPin, OUTPUT);
	servoX.attach(xPin, 1500,1600);
	servoY.attach(yPin, 1500,1600);
	wakeUp();  //Wake up our roomba bot

	
	//FunctionPointer testFunc = &test;  //GRACE: Declare each task this way
	//queueMaker(testFunc, 0);  //GRACE: Put tasks in queue like this.  The int is for it's order.

	//task queue:
	FunctionPointer btle = &btListen;
	queueMaker(btle, 0);
	FunctionPointer getDriving = &driving;
	queueMaker(getDriving, 1);
	FunctionPointer servos = &servoing;
	queueMaker(servos, 2);
	FunctionPointer lasering = &laser;
	queueMaker(lasering, 3);
	FunctionPointer gettingShot = &lightSensor;
	queueMaker(gettingShot, 4);
	FunctionPointer riverChecking = &river;
	queueMaker(riverChecking, 5);
	
	timerMaker();

	Serial.begin(19200); //console
	Serial1.begin(19200);  //roomba
	Serial2.begin(19200); //bluetooth
	Serial1.write(128); //start the roomba
	Serial1.write(132); //full mode
	Serial1.write(141);
	Serial1.write(2);
	//idler();
}

int queueMaker(FunctionPointer func, int offset){
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
	TCCR5B |= (1 << CS52) | (0 << CS51) | (0 << CS50);
	TIMSK5 |= (1 << TOIE5);

}

 void btListen(){
	digitalWrite(51, digitalRead(51) ^ 1);
	static boolean recvInProgress = false;
	static byte ndx = 0;
	char startMarker = '<';
	char endMarker = '>';
	char rc;

	while (Serial2.available() > 0 && newData == false) {
		rc = Serial2.read();
		//Serial.print(rc);
		if (recvInProgress == true) {
			if (rc != endMarker) {
				receivedChars[ndx] = rc;
				ndx++;
				if (ndx >= numChars) {
					ndx = numChars - 1;
				}
			}
			else {
				receivedChars[ndx] = '\0';
				recvInProgress = false;
				ndx = 0;
				newData = true;
			}
		}

		else if (rc == startMarker) {
			recvInProgress = true;
		}
	}
	//Serial.println();

	if (newData == true) {
		strcpy(tempChars, receivedChars);
		parseData();

		newData = false;
	}
			showParsedData();
	digitalWrite(51, digitalRead(51) ^ 1);
}


void parseData() {
	char * strtokIndx;
	strtokIndx = strtok(tempChars,",");
	xVal = atoi(strtokIndx);
	strtokIndx = strtok(NULL, ",");
	yVal = atoi(strtokIndx);
	strtokIndx = strtok(NULL, ",");
	trigger = atoi(strtokIndx);
	strtokIndx = strtok(NULL, ",");
	velocity = atoi(strtokIndx);
	strtokIndx = strtok(NULL, ",");
	radius = atoi(strtokIndx);
}

void showParsedData() {
	Serial.print("X:");
	Serial.print(xVal);
	Serial.print(" Y:");
	Serial.print(yVal);
	Serial.print(" Z:");
	Serial.print(trigger);
	Serial.print(" V:");
	Serial.print(velocity);
	Serial.print(" R:");
	Serial.println(radius);
}

void driving(){
			//driveTimer = 0;  //uncomment for testing
	if (driveTimer < 1200){
		clamp(velocity, -500, 500);
		clamp(radius, -2000, 2000);
		if (oldVel != velocity || oldRad != radius){
			oldVel = velocity;
			oldRad = radius;
			Serial1.write(137);
			Serial1.write(velocity >> 8);
			Serial1.write(velocity);
			Serial1.write(radius >> 8);
			Serial1.write(radius);
		}
			}
	if (driveTimer >= 1200 && driveTimer < 2384) {
		stopDrive();
	}
	if (driveTimer >=2384){
		driveTimer = 0;
	}
}

void servoing(){
	digitalWrite(47, digitalRead(47) ^ 1);
	if (oldX != xVal){
		servoX.write(xVal);
	}
	if (oldY != yVal){
		servoY.write(yVal);
	}
	digitalWrite(47, digitalRead(47) ^ 1);
}

void lightSensor(){
	photoVal = analogRead(photoSensor);
	if (photoVal > photoThreshold){
		if (shot > 40){
			while(1){ //We're dead!!
				noInterrupts();
			}
			} else {
			shotFlag = true;
		}
		} else {
		shotFlag = false;
	}
}

void river(){
	Serial1.write(142);
	Serial1.write(13);
	while(Serial1.available() > 0){
		riverFlag = Serial1.read();
	}
	while(riverFlag){
		noInterrupts();  //We're dead!!
	}
}

void wakeUp (void)
{
	digitalWrite(ddPin, HIGH);
	delay(100);
	digitalWrite(ddPin, LOW);
	delay(500);
	digitalWrite(ddPin, HIGH);
	delay(2000);
	for(int i = 0; i < 3;i++){
		digitalWrite(ddPin, LOW);
		delay(50);
		digitalWrite(ddPin, HIGH);
		delay(50);
	}
}

void stopDrive(void)
{
	Serial1.write(137);
	Serial1.write(0 >> 8);
	Serial1.write(0);
	Serial1.write(0 >> 8);
	Serial1.write(0);
}

void laser(){
	if(laserTimer < 200 && trigger){
		digitalWrite(laserPin, HIGH);
		} else {
		digitalWrite(laserPin, LOW);
	}
}

void champions(){
	Serial1.write(140);
	Serial1.write(2);
	Serial1.write(8);
	Serial1.write(98);
	Serial1.write(255);
	Serial1.write(97);
	Serial1.write(32);
	Serial1.write(98);
	Serial1.write(32);
	Serial1.write(97);
	Serial1.write(100);
	Serial1.write(81);
	Serial1.write(100);
	Serial1.write(78);
	Serial1.write(75);
	Serial1.write(83);
	Serial1.write(32);
	Serial1.write(78);
	Serial1.write(100);
	
}


ISR(TIMER0_COMPA_vect)          // Fires off a task each time our period comes up.  I need to fix this crap

{
	test();
	(*funcs[funcCounter])();
	funcCounter++;
	if (funcCounter == maxFuncs)
	funcCounter = 0;
}

ISR(TIMER5_OVF_vect)        // Timer for counting off the laser and the 30 second freeze
{
	driveTimer++;
	if (trigger){
		laserTimer++;
	}
	if (shotFlag){
		shot++;
	}
	TCNT5 = 259000;
}

int main(void) {
	init();
	setup();
	while (1)
	{
	}
}