/*
 * two_testy.c
 *
 * Created: 12/1/2019 6:10:39 PM
 * Author : Christian Legaspino
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"

//--------Find GCD function --------------------------------------------------
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
//--------End find GCD function ----------------------------------------------
//--------Task scheduler data structure---------------------------------------
typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	unsigned char active;
	int (*TickFct)(int);
} task;
//--------End Task scheduler data structure-----------------------------------
unsigned char cnt = 0;
unsigned char limit = 10;
unsigned char tmp = 0x00;
enum blink {init, on, off};
int blinkFct(int state) {
	switch (state) { // Transitions
		case init:
			state = on; break;
		case on:
			if(USART_HasReceived(0)){
				tmp = USART_Receive(0);
				USART_Flush(0);
			}
			else {
				tmp = 0x00;
				USART_Flush(0);
			}
			PORTB = tmp;
			state = on;
			break;
			
// 			if(tmp == 0xFF){
// 				limit = 5;
// 			}
// 			if(cnt >= limit){
// 				state = off;
// 				break;
// 			} else {
// 				state = on;
// 				break;
// 			}
		case off:
			if(USART_HasReceived(0)){
				tmp = USART_Receive(0);
				USART_Flush(0);
			}
			if(tmp == 0xFF){
				limit = 5;
			}
			if(cnt >= limit){
				state = on;
				break;
			} else {
				state = off;
				break;
			}
		default:
			state = init; break;
	}
	switch (state) { // Actions
		case init:
			break;
		case on:
			//PORTB = 0x01; break;
		case off:
			PORTB = 0x00; break;
		default:
			break;
	}
	return state;
}

// Implement scheduler code from PES.
int main()
{
	
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0x00; PORTD = 0xFF;

	// Period for the tasks
	unsigned long int SMTick1_calc = 100;

	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(tmpGCD, SMTick1_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;

	//Declare an array of tasks
	static task task1;
	task *tasks[] = {&task1};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.active = 0x01;
	task1.TickFct = &blinkFct;//Function pointer for the tick.

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();

	unsigned short i;
	while(1) {
		for ( i = 0; i < numTasks; i++ ) {
			if(tasks[i]->active){
				if ( tasks[i]->elapsedTime >= tasks[i]->period ) {
					tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
					tasks[i]->elapsedTime = 0;
				}
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}