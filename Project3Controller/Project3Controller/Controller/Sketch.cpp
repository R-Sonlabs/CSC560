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
ISR(TIMER0_COMPA_vect );
ISR(TIMER5_OVF_vect );

#define period 10000  //GRACE: This is how frequently the timer fires off the next task
#define testPin 52 //GRACE: just use this for when you need to test if a task is firing
#define maxFuncs 6 //GRACE: increment this for every task you add
#define xPin A0
#define yPin A1
#define vPin A2
#define rPin A3
#define zPin 7

FunctionPointer funcs[maxFuncs];

int funcCounter;
int velocity;
int radius;
int xVal;
int yVal;
bool trigger;
bool idleFlag = true;

void setup() {
  pinMode(testPin, OUTPUT);
  pinMode(zPin, INPUT_PULLUP);
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
  int rawV = 525;//analogRead(vPin);
  if (rawV <= 500 && rawV >= 250){
    velocity = -100;
  }
  if (rawV < 250) {
    velocity = -200;
  }
  if (rawV >=550 && rawV <= 800){
    velocity = 100;
  }
  if (rawV > 800){
    velocity = 200;
  }
  if (rawV > 500 && rawV < 550){
    velocity = 32768;
  }
  
}

void getRadius(){
  int rawR = analogRead(rPin);
  if (rawR <= 700 && rawR >= 500){
    radius = -1000;
  }
  if (rawR < 500) {
    radius = -2000;
  }
  if (rawR >= 980 && rawR <= 1000){
    radius = 1000;
  }
  if (rawR > 1000){
    radius = 2000;
  }
  if (rawR > 970 && rawR < 980){
    radius = 32768;
  }
}

void getXval(){
  xVal = constrain(xVal, 15, 160);
  int rawX = analogRead(xPin);
  if (rawX > 1000){
    xVal = xVal + 3;
  }
  if (rawX <= 1000 && rawX > 550){
    xVal = xVal + 1;
  }
  if (rawX <= 500 && rawX > 10) {
    xVal = xVal - 1;
  }
  if (rawX < 10) {
    xVal = xVal - 3;
  }
  if (rawX > 500 && rawX <= 550) {
    xVal = xVal;
  }  
}

void getYval(){
  yVal = constrain(yVal, 15, 160);
  int rawY = analogRead(yPin);
  if (rawY > 1000){
    yVal = yVal + 3;
  }
  if (rawY <= 1000 && rawY > 550){
    yVal = yVal + 1;
  }
  if (rawY <= 500 && rawY > 10) {
    yVal = yVal - 1;
  }
  if (rawY < 10) {
    yVal = yVal - 3;
  }
  if (rawY > 500 && rawY <= 550) {
    yVal = yVal;
  }  
}

void btTransmit(){
  Serial1.print("<");
  Serial1.print(xVal);
  Serial1.print(",");
  Serial1.print(yVal);
  Serial1.print(",");
  Serial1.print(trigger);
  Serial1.print(",");
  Serial1.print(velocity);
  Serial1.print(",");
  Serial1.print(radius);
  Serial1.print(">");

  Serial.print("<");
  Serial.print(xVal);
  Serial.print(",");
  Serial.print(yVal);
  Serial.print(",");
  Serial.print(trigger);
  Serial.print(",");
  Serial.print(velocity);
  Serial.print(",");
  Serial.print(radius);
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
