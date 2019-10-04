/*
 * Christian Legaspino (clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 1 Exercise 2
 * Lab Description: Press button at certain time with alternating LEDs. Display score on LCD.
 * 
 * I acknowledge all content contained herein, excluding template 
 *		or example code, is my own original work.
 */ 

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
unsigned char s = 0x00;
#define a (PINA & 0x01)
unsigned char b_light = 0x00;
unsigned char t_light = 0x00;
unsigned char point = 0x00;
//--------End Shared/Global Variables------------------------------------------------

//--------User defined FSMs---------------------------------------------------
//Button
enum SM1_States{start, on, off, p_on, p_off};
int SM1Tick(int state){
	
	LCD_Cursor(1);
	LCD_WriteData('0' + point);
	
	switch(state){
		case start:
		state = off; break;
		case on:
		state = !a ? p_off : on; break;
		case off:
		state = !a ? p_on : off; break;
		case p_on:
		state = !a ? state : on; break;
		case p_off:
		state = !a ? state : off; break;
		default:
		state = start; break;
	}
	switch(state){
		case on:
			s = 0x01;
			break;
		case off:
			s = 0x00; break;
		case p_on:
			if(t_light == 0x02){
				point += 1;
				}else{
				if(point > 0){point -= 1;}
			}break;
		default:
			break;
	}
	return state;
}


//Blinking Light
enum SM2_States{b_start, b_on, b_off};
int SM2Tick(int state){
	switch(state){
		case b_start:
			state = b_on; break;
		case b_on:
			state = s ? state : b_off; break;
		case b_off:
			state = s ? state : b_on; break;
		default:
			state = b_start; break;
	}
	switch(state){
		case b_on:
			b_light = 0x08; break;
		case b_off:
			b_light = 0x00; break;
	}
	return state;
}


//Three LED
enum SM3_States{t_start, t1, t2, t3};
int SM3Tick(int state){
	switch(state){
		case t_start:
			state = t1; break;
		case t1:
			state = s ? state : t2; break;
		case t2:
			state = s ? state  : t3; break;
		case t3:
			state = s ? state : t1; break;
		default:
			state = t_start; break;
	}
	switch(state){
		case t1:
		t_light = 0x01; break;
		case t2:
		t_light = 0x02; break;
		case t3:
		t_light = 0x04; break;
	}
	return state;
}

// void ScoreKeeper(unsigned char points)
// {
// 	switch(points)
// 	{
// 		case 0: PORTD = 0x00; break;
// 		case 1: PORTD = 0x01; break;
// 		case 2: PORTD = 0x03; break;
// 		case 3: PORTD = 0x07; break;
// 		case 4: PORTD = 0x0F; break;
// 		case 5: PORTD = 0x1F; break;
// 		default: PORTD = PORTD;
// 	}
// }
// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	LCD_init();
	LCD_ClearScreen();

	//LCD_DisplayString(1, "Hello World1");

	// Period for the tasks
	unsigned long int SMTick1_calc = 150;
	unsigned long int SMTick2_calc = 1000;
	unsigned long int SMTIck3_calc = 500;

	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	tmpGCD = findGCD(tmpGCD, SMTIck3_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTIck2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTIck3_calc/GCD;


	//Declare an array of tasks
	static task task1, task2, task3;
	task *tasks[] = {&task1, &task2, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &SM1Tick;//Function pointer for the tick.

	// Task 2
	task2.state = -1;
	task2.period = SMTIck2_period;
	task2.elapsedTime = SMTIck2_period;
	task2.TickFct = &SM2Tick;

	//Task 3
	task3.state = -1;
	task3.period = SMTick3_period;
	task3.elapsedTime = SMTick3_period;
	task3.TickFct = &SM3Tick;

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	
	unsigned short i;
	while(1) {
		for ( i = 0; i < numTasks; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
			PORTB = b_light | t_light;
			
		}
		//ScoreKeeper(point);
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}

