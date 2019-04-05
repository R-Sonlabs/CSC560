/*
  Roomba Controller
  CSC560 - Project 3
  UVIC, Spring 2019
  
  Matt Richardson & Grace Liu
  Team 9
*/

#include <Arduino.h>
#include <Servo.h>

typedef void (*FunctionPointer)();

int queueMaker(FunctionPointer func, int offset);
int test();
void idler();
void timerMaker();
void clockT(int delayTime);
void btListen();
void parseData();
void showParsedData();
void drive(int vel, int rad);
void servoing();
void river();
void wakeUp(void );
void stopDrive(void );
void laser();
void champions();
void lightSensor();
void startFull();
void playSound(int num);
void escape();
void clockTest();
ISR(TIMER0_COMPA_vect );
ISR(TIMER5_OVF_vect );


#define period 100  //GRACE: This is how frequently the timer fires off the next task
#define testPin 52 //GRACE: just use this for when you need to test if a task is firing
#define maxFuncs 5 //GRACE: increment this for every task you add
#define ddPin 12
#define laserPin 9
#define xPin 2
#define yPin 5
#define photoSensor A8
#define photoThreshold 500
#define clamp(value, min, max) (value < min ? min : value > max ? max : value)
#define btListenPin 53
#define servoingPin 51
#define laserPin 49
#define lightSensorPin 47
#define riverPin 45






FunctionPointer funcs[maxFuncs];

int clockTimer = 0;  //Increments 50.33ms - Useful for generally timing stuff.
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
bool button1;
bool button2;
bool button3;
bool oldTrigger;
bool autonomousFlag = false;
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
//  pinMode(xPin, OUTPUT);
//  pinMode(yPin, OUTPUT);
  pinMode(btListenPin, OUTPUT);
  pinMode(servoingPin, OUTPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(lightSensorPin, OUTPUT);
  pinMode(riverPin, OUTPUT);
  pinMode(laserPin, OUTPUT);
  servoX.attach(xPin);
  servoY.attach(yPin);
  wakeUp();  //Wake up our roomba bot

  //FunctionPointer testFunc = &test;  //GRACE: Declare each task this way
  //queueMaker(testFunc, 0);  //GRACE: Put tasks in queue like this.  The int is for it's order.

  //task queue:
  FunctionPointer btle = &btListen;
  queueMaker(btle, 0);
  FunctionPointer servos = &servoing;
  queueMaker(servos, 1);
  FunctionPointer lasering = &laser;
  queueMaker(lasering, 2);
  FunctionPointer gettingShot = &lightSensor;
  queueMaker(gettingShot, 3);
  FunctionPointer riverChecking = &river;
  queueMaker(riverChecking, 4);
  


  Serial.begin(19200); //console
  Serial1.begin(19200);  //roomba
  Serial2.begin(19200); //bluetooth
  startFull();
  timerMaker();
  stopDrive();
  //playSound(3);
  //motorSquareTest();
  //idler();
}

int queueMaker(FunctionPointer func, int offset) {
  funcs[offset] = func;
}

int test() { //Just a testing function to call for debbugging
  digitalWrite(testPin, digitalRead(testPin) ^ 1);
}

void idler() {

  while (1) {
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

void btListen() {
  digitalWrite (btListenPin, digitalRead(btListenPin) ^ 1);
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial2.available() > 0 && newData == false) {
    rc = Serial2.read();
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
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    parseData();
    showParsedData();
    newData = false;
  }

}


void parseData() {
  char * strtokIndx;
  strtokIndx = strtok(tempChars, ",");
  xVal = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  yVal = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  trigger = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  velocity = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  radius = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  button1 = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  button2 = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  button3 = atoi(strtokIndx);
  drive(velocity, radius);
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
  digitalWrite (btListenPin, digitalRead(btListenPin) ^ 1);
}

void drive(int vel, int rad) {
  driveTimer = 0;  //uncomment for testing
  if (driveTimer < 1200) {
    clamp(velocity, -500, 500);
    clamp(radius, -2000, 2000);
    if (oldVel != velocity || oldRad != radius){
      oldVel = velocity;
        oldRad = radius;
    Serial1.write(137);
    Serial1.write(vel >> 8);
    Serial1.write(vel);
    Serial1.write(rad >> 8);
    Serial1.write(rad);
    }
  }
  if (driveTimer >= 1200 && driveTimer < 2384) {
    stopDrive();
  }
  if (driveTimer >= 2384) {
    driveTimer = 0;
  }
}

void servoing() {
  digitalWrite(servoingPin, digitalRead(servoingPin) ^ 1);
  servoX.write(xVal);
  Serial.println(xVal);
  servoY.write(yVal);
  digitalWrite(servoingPin, digitalRead(servoingPin) ^ 1);
}

void lightSensor() {
  digitalWrite(lightSensorPin, digitalRead(lightSensorPin) ^ 1);
  photoVal = analogRead(photoSensor);
  if (photoVal > photoThreshold) {
    if (shot > 40) {
      while (1) { //We're dead!!
        noInterrupts();
        playSound(2);
      }
      } else {
      shotFlag = true;
    }
    } else {
    shotFlag = false;
  }
  digitalWrite(lightSensorPin, digitalRead(lightSensorPin) ^ 1);
}

void river() {
  digitalWrite(riverPin, digitalRead(riverPin) ^ 1);
  Serial1.write(142);
  Serial1.write(13);
  while (Serial1.available() > 0) {
    riverFlag = Serial1.read();
  }
  if(riverFlag){
    escape();
  }
  digitalWrite(riverPin, digitalRead(riverPin) ^ 1);
}

void wakeUp (void)
{
  digitalWrite(ddPin, HIGH);
  delay(100);
  digitalWrite(ddPin, LOW);
  delay(500);
  digitalWrite(ddPin, HIGH);
  delay(2000);
  for (int i = 0; i < 3; i++) {
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

void startFull()
{
  Serial1.write(128);  
  Serial1.write(132); 
  delay(1000);
}

void laser() {
  if (laserTimer < 200 && trigger) {
    digitalWrite(laserPin, HIGH);
    } else {
    digitalWrite(laserPin, LOW);
  }
}

void playSound (int num)
{
  switch (num)
  {
    case 1:
    Serial1.write("\x8c\x01\x04\x42\x20\x3e\x20\x42\x20\x3e\x20");
    Serial1.write("\x8d\x01");
    break;

    case 2:
    Serial1.write("\x8c\x01\x01\x3c\x20");
    Serial1.write("\x8d\x01");
    break;

    case 3:
    Serial1.write("\x8c\x01\x01\x48\x20");
    Serial1.write("\x8d\x01");
    break;
  }
}

void champions() {
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
  (*funcs[funcCounter])();
  funcCounter++;
  if (funcCounter == maxFuncs)
  funcCounter = 0;
}

ISR(TIMER5_OVF_vect)        // Timer for counting off the laser and the 30 second freeze
{
  clockTimer++;
  driveTimer++;
  if (trigger) {
    laserTimer++;
  }
  if (shotFlag) {
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

void clockT(int delayTime){
  int dividedTimer = (delayTime/50.33) + clockTimer;
  test();
  while(clockTimer < dividedTimer){
    Serial.println(clockTimer);
  }
  test();
}

void escape(){
  autonomousFlag = true;
  stopDrive();
  drive(-200, 0);
  playSound(3);
  clockT(1000);
  autonomousFlag = true;
}

// void motorSquareTest(void)
// {
//  drive (100, 0);
//  clockT(2000);
//  stopDrive();
//  drive (60, 1);
//  clockT(4000);
//  drive (100, 0);
//  clockT(2000);
//  stopDrive();
//  drive (60, 1);
//  clockT(4000);
//  drive (100, 0);
//  clockT(2000);
//  stopDrive();
//  drive (60, 1);
//  clockT(4000);
//  drive (100, 0);
//  clockT(2000);
//  stopDrive();
//  drive (60, 1);
//  clockT(4000);
//  stopDrive();
// }



