/*
 * Christian Legaspino (clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 2 Exercise 2
 * Lab Description: Add a button to toggle micro controller between master and servant. 
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
#define s (PINB & 0x01)
//--------End Shared/Global Variables------------------------------------------------

//--------User defined FSMs---------------------------------------------------

enum SM1_States{start1, wait, on, off};
int SM1Tick(int state){
	
	switch(state){
		case start1:
			state = on; break;
		case wait:
			if(s){state = on; break;}
				else{state = wait; break;}
		case on:
			if(s){state = off; break;}
				else{state = wait; break;}
		case off:
			if(s){state = on; break;}
				else{state = wait; break;}
		default:
			state = start1; break;
	}
	switch(state){
		case start1:
			break;
		case wait:
			PORTA = 0x00;
			PORTC = 0x00;
			USART_Send(PORTA, 1);
			break;
		case on:
			PORTA = 0x01;
			PORTC = 0x01;
			USART_Send(PORTA, 1);
			break;
		case off:
			PORTA = 0x00;
			PORTC = 0x01;
			USART_Send(PORTA, 1);
			break;
		default:
			break;
	}
	return state;
}

enum SM2_States{start2, follow};
int SM2Tick(int state){

	switch(state){
		case start2:
			state = follow; break;
		case follow:
			state = follow; break;
		default:
			state = start2; break;
	}
	switch(state){
		case start2:
			break;
		case follow:
			if(!s){
				PORTA = USART_Receive(0); break;
			}else{
				break;
			}
			break;
		default:
			break;
	}
	
}
// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;

	

	// Period for the tasks
	unsigned long int SMTick1_calc = 500;
	unsigned long int SMTick2_calc = 10;
	

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
	
	task2.state = -1;
	task2.period = SMTick2_period;
	task2.elapsedTime = SMTick2_period;
	task2.TickFct = &SM2Tick;


	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	
	initUSART(0);
	initUSART(1);
	
	unsigned short i;
	while(1) {
		
		USART_Flush(0);
		
		for ( i = 0; i < numTasks; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
			/*PORTA = 0x01;*/
			
			if(s){
				USART_Send(PORTA, 1);
			}
			
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}