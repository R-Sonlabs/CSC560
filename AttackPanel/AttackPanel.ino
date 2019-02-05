#include <Servo.h>

int x_pwm,y_pwm;
int x_servo_pin = 6;
int y_servo_pin = 7;
int Laser = 5;
int X_read;
int Y_read;
char L_read;

Servo servo_x, servo_y;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  servo_x.attach(x_servo_pin, 1500, 1600);
  servo_y.attach(y_servo_pin, 1500, 1600);
  //servo_x.write(110);
  pinMode (Laser, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  readBluetooth();
  Serial.println(X_read);
  x_pwm = constrain(X_read, 110, 180);
  y_pwm = constrain(Y_read, 0, 180);
  servo_x.write(x_pwm);
  servo_y.write(y_pwm);
  if(L_read == '1') {
    //Serial.println(L_read);
    digitalWrite(Laser, HIGH);
  }else {
    digitalWrite(Laser, LOW);
  }
}

void readBluetooth()
{
  int nextRead;
  char curRead;
  if(Serial1.available() > 0){
    //Serial.write(Serial1.read());
    curRead = Serial1.read();
    //Serial.println(curRead);
    if(curRead == 'X'){
      delay(50);
      nextRead = Serial1.read();
      while (nextRead != 'Y'){
        nextRead += Serial1.read();
      }
      X_read = nextRead;
    }
    if(curRead == 'Y'){
      delay(50);
      nextRead = Serial1.read();
      while (nextRead != 'L'){
        nextRead += Serial1.read();
      }
      Y_read = nextRead;
    }
    if(curRead == 'L'){
      delay(50);
      L_read = (char)Serial1.read();
    }
  }
}
  
