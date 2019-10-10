/*
 * Christian Legaspino (clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 2 Exercise 1
 * Lab Description: 
 * 
 * I acknowledge all content contained herein, excluding template 
 *		or example code, is my own original work.
 */ 
/////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/interrupt.h>
#include <bit.h>
#include <timer.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdbool.h>
#include <alloca.h>
#include <stdlib.h>
#include "io.c"
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
	int (*TickFct)(int);
} task;

//--------End Task scheduler data structure-----------------------------------
//--------Shared/Global Variables----------------------------------------------------

//--------End Shared/Global Variables------------------------------------------------

//--------User defined FSMs---------------------------------------------------
//Button
enum SM1_States{start, on, off};
int SM1Tick(int state){
	
	switch(state){
		case start:
			state = on; break;
		case on:	
			state = off; break;
		case off:
			state = on; break;
		default:
			state = start; break;
	}
	switch(state){
		case start:
			break;
		case on:
			PORTA = 0x01; 
			
			if(!USART_IsSendReady(0)){PORTB = 0x01;}
			else{
			USART_Send(PORTA,0);
			PORTB = 0x02;
			}
			break;
		case off:
			PORTA = 0x00; 
			
			if(!USART_IsSendReady(0)){PORTB = 0x01;}
			else{
			USART_Send(PORTA,0);
			PORTB = 0x02;
			}
			break;
		default:
			break;
	}
	return state;
}


// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	

	// Period for the tasks
	unsigned long int SMTick1_calc = 500;
	

	//Calculating GCD
	unsigned long int tmpGCD = 1;
// 	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
// 	tmpGCD = findGCD(tmpGCD, SMTIck3_calc);

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
	task1.TickFct = &SM1Tick;//Function pointer for the tick.


	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	
	unsigned short i;
	while(1) {
		
		//initialize USART
		initUSART(0);
		USART_Flush(0);
		
		for ( i = 0; i < numTasks; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
			/*PORTA = 0x01;*/
			USART_Send(PORTA,0);
			
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}