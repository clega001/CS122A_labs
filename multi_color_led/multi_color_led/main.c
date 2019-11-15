/*
 * multi_color_led.c
 *
 * Created: 11/7/2019 6:33:05 PM
 * Author : Christian Legaspino
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer.h>
#include <util/delay.h>
#include "pwm.c"

unsigned char m = 0x00;
unsigned char n = 0x00;
unsigned short t = 0x0000;
unsigned char temp1 = 0x00;
unsigned char temp2 = 0x00;
//-----------------------------------------------------------------
unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
	unsigned long int c;
	while(1){
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}

typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

// void ADC_init() {
// 	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
// 	ADMUX |= (1 << REFS0);
// }

//taken from http://extremeelectronics.co.in/avr-tutorials/using-adc-of-avr-microcontroller/
void ADC_init()
{
	ADMUX=(1<<REFS0);                         // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
}
uint16_t ReadADC(uint8_t ch)
{
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	
	ADMUX|=ch;
	
	// I added here: allows time to switch between analog channels
	_delay_ms(2);

	//Start Single conversion
	ADCSRA|=(1<<ADSC);

	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!

	ADCSRA|=(1<<ADIF);

	return(ADC);
}

//interrupt used from: https://newbiehack.com/MicrocontrollersADCReadingMultipleAnalogVolageSources.aspx
// ISR(ADC_vect){
// 	switch(ADMUX){
// 		case 0x40:
// 			break;
// 		case 0x41:
// 			break;
// 		default:
// 			break;
// 	}
// 	
// 	ADCSRA|=(1<<ADSC);
// }
//-----------------------------------------------------------------
enum SM1_States{start, one} state;
int SM1Tick(int state){//Transition
	switch(state){
		case start:
			state = one; break;
		case one:
			state = one; break;
		default:
			state = start; break;
	}
	switch(state){//Action
		case start:
			break;
		case one:
			m = ReadADC(0);
			
			if(m > 240){
				PORTB = 0x01;
			}
			else if(m > 100){
				PORTB = 0x02;
			}
			else {
				PORTB = 0x04;
			}
			break;
		default:
			break;
	}
	return state;
}

enum SM2_States {init, two} state_2;
int SM2Tick(){
	switch(state_2){//Transition
		case init:
			state_2 = two; break;
		case two:
			state_2 = two; break;
		default:
			state_2 = init; break;
	}
	switch(state_2){//Action
		case init:
			break;
		case two:
			n = ReadADC(1);
			
			if(n > 240){
				PORTD = 0x01;
			}
			else if(n > 100){
				PORTD = 0x02;
			}
			else {
				PORTD = 0x04;
			}
			break;
		default:
			break;
	}
	return state_2;
}
//---------------------------------------------------------------------------
// Implement scheduler code from PES.
int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	// Period for the tasks
	unsigned long int SMTick1_calc = 1;
	unsigned long int SMTick2_calc = 1;
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;

	//Declare an array of tasks
	static task task1, task2;
	task *tasks[] = {&task1, &task2};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &SM1Tick;//Function pointer for the tick.

	// Task 2
	task2.state = -1;
	task2.period = SMTick2_period;
	task2.elapsedTime = SMTick2_period;
	task2.TickFct = &SM2Tick;

	TimerSet(GCD);
	TimerOn();

	ADC_init();
	
	unsigned short i;
	while(1) {
		for ( i = 0; i < numTasks; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}