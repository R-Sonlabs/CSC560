/*
 * ArduinoCore.cpp
 *
 * Created: 2019-04-03 03:43:08
 * Author : timbi
 */ 

#include <avr/io.h>

#define MIN_PULSE_WIDTH       544     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      2400     // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define MAX_SERVOS           2
#define SERVO_MIN() (MIN_PULSE_WIDTH - this->min * 4)  // minimum value in uS for this servo
#define SERVO_MAX() (MAX_PULSE_WIDTH - this->max * 4)  // maximum value in uS for this servo 
#define usToTicks(_us)    ((clockCyclesPerMicrosecond() * _us) / 16)      // converts microseconds to tick
#define ticksToUs(_ticks) (((unsigned) _ticks * 16) / clockCyclesPerMicrosecond())   // converts from ticks back to microseconds

typedef struct {
  uint8_t Pin;
  unsigned int ticks;
} servo_t;

class Servo{
	public:
		Servo();
		uint8_t attach(int pin, int min, int max);
		void write(int val);
		int read();

	private:
		uint8_t servoIndex;               // index into the channel data for this servo, 0:x, 1:y
		int8_t min;
		int8_t max;
};

static servo_t servos[MAX_SERVOS];
uint8_t servo_count = 0;

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
	if(servo_count < MAX_SERVOS){
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
	byte channel = this->servoIndex;
	if( (channel >= 0) && (channel < MAX_SERVOS)){  
		if( value < SERVO_MIN()){
			value = SERVO_MIN();
		}else if( value > SERVO_MAX()){
			value = SERVO_MAX(); 
		}
		value = (value-5);
		value = usToTicks(value);  // convert to ticks after compensating for interrupt overhead
	    servos[channel].ticks = value;
	}
}

int read(){
	unsigned int pulsewidth;
  	if (this->servoIndex < MAX_SERVOS)
    	pulsewidth = ticksToUs(servos[this->servoIndex].ticks) + 5;
  	else
    	pulsewidth  = 0;
  	return pulsewidth;
}