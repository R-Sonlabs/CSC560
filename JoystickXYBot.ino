#include <Servo.h>;
int JoyStick_X_A = A0;
int JoyStick_Y_A = A1;
int JoyStick_Z_A = 4;
int x_filtered_A, y_filtered_A;
int x_pwm,y_pwm;
int x_servo_pin = 6;
int y_servo_pin = 7;
float alpha = .9;
int Laser = 5;
int delay1 = 20;
int delay2 = 50; 

Servo servo_x, servo_y;
void setup ()
{
  servo_x.attach(x_servo_pin, 1000, 2000);
  servo_y.attach(y_servo_pin, 1000, 2000);
  pinMode (JoyStick_X_A, INPUT);
  pinMode (JoyStick_Y_A, INPUT);
  pinMode (JoyStick_Z_A, INPUT_PULLUP);
  pinMode (Laser, OUTPUT);
  Serial.begin (9600);
  x_filtered_A = analogRead(JoyStick_X_A);
  y_filtered_A = analogRead(JoyStick_Y_A);
}
void loop ()
{
  time1();
  delay (delay1);
  int z_A = digitalRead(JoyStick_Z_A);
  x_pwm = constrain(x_pwm, 15, 160);
  y_pwm = constrain(y_pwm, 0, 180);
  Serial.print(x_pwm);
  Serial.print(", ");
  Serial.print(y_pwm);
  Serial.print(", ");
  Serial.println(z_A);
  servo_x.write(x_pwm);
  servo_y.write(y_pwm);
  if(z_A == 0) {digitalWrite(Laser, HIGH);} else {digitalWrite(Laser, LOW);}
}

void time1(){
    
    int x_raw_A = analogRead(JoyStick_X_A);
    x_filtered_A = (alpha*x_raw_A) + ((1-alpha)*x_filtered_A);  
    if(x_filtered_A > 1000){x_pwm = x_pwm + 3;} 
    else if(x_filtered_A <= 1000 && x_filtered_A > 510){x_pwm = x_pwm + 1;}
    else if(x_filtered_A <= 490 && x_filtered_A > 10){x_pwm = x_pwm - 1;}
    else if(x_filtered_A <= 10){x_pwm = x_pwm - 3;}

    int y_raw_A = analogRead(JoyStick_Y_A);
    y_filtered_A = (alpha*y_raw_A) + ((1-alpha)*y_filtered_A);  
    if(y_filtered_A < 10){y_pwm = y_pwm + 3;} 
    else if(y_filtered_A >= 10 && y_filtered_A < 525){y_pwm = y_pwm + 1;}
    else if(y_filtered_A >= 535 && y_filtered_A < 1000){y_pwm = y_pwm - 1;}
    else if(y_filtered_A >= 1000){y_pwm = y_pwm - 3;}
    
  

}

  
