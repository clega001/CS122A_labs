/*
 * Christian Legaspino clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 6 Exercise 1
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
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
#include <avr/sleep.h>
#include <unistd.h>
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
	unsigned char active;
	int (*TickFct)(int);
} task;
//--------End Task scheduler data structure-----------------------------------

#include <avr/io.h>
#define buttons (~PINA & 0x06)
const unsigned char Inc = (1 << PINA0); // Which pin is the increment button
const unsigned char Dec = (1 << PINA1); // Which pin is the decrement button
const unsigned char Reset = (1<<PINA0) | (1<<PINA1);
const unsigned char Display = (1<<PINA2);

#define b1 (~PINA & 0x01)
#define b2 (~PINA & 0x02)
#define b3 (~PINA & 0x04)

static task task1, task2, task3;
unsigned char t = 0;

unsigned char count = 3;
enum IncDec_States { Start, Wait, IncHold, DecHold, ResetHold };
void updateCount(int);
const unsigned char InitialCount = 3;
int IncDec_tick(int state) {
	switch (state) { // Transitions
		case Start:
		count = InitialCount;
		state = Wait;
		break;
		case Wait:
		if (b1 && !b2) {
			updateCount(1);
			state = IncHold;
			} else if (!b1 && b2) {
			updateCount(-1);
			state = DecHold;
			} else if (b1 && b2) {
			count = 0;
			state = ResetHold;
			} else {
			state = Wait;
		}
		break;
		case IncHold:
		
		//task2.active = 0x01;
		
		if (b1 && !b2) {
			state = IncHold;
			} else if (b1 && b2) { // Intentional bug. Leave in.
			PORTD = 0xFF;
			for(;;);
			} else {
			state = Wait;
		}
		break;
		case DecHold:
		if (!b1 && b2) {
			state = DecHold;
			} else if(b1 && b2){
				state = ResetHold;
			} else {
			state = Wait;
		}
		break;
		case ResetHold:
		if (b1 && b2) {
			state = ResetHold;
			} else {
			state = Wait;
		}
		break;
		default:
		state = Wait;
		break;
	}
	switch (state) { // Actions
		case Start:
		case Wait:
		case IncHold:
		case DecHold:
		case ResetHold:
		break;
	}
	return state;
}

enum Lcd_States {init, display} lcd_state;
int LCD_tick(){
	switch(lcd_state){
		case init:
			lcd_state = display; break;
		case display:
			lcd_state = display; break;
		default:
			lcd_state = init; break;
	}
	switch(lcd_state){
		case init:
			break;
		case display:
			if(t >= 3){
				LCD_ClearScreen();
				task2.active = 0x00;
				PORTB = 0x00; 
				t = 0;
				break;
			}
			else{
				t += 1;
				/*LCD_DisplayString(1, "Hey Chris");*/
				LCD_Cursor(1);
				LCD_WriteData(count + '0');
			}
			break;
		default:
			break;
	}
	return lcd_state;
}

enum Wake_States{w_init, listen} w_state;
int Wake_tick(){
	switch(w_state){
		case w_init:
			w_state = listen; break;
		case listen:
			w_state = listen; break;
		default:
			w_state = w_init; break;
	}
	switch(w_state){
		case w_init:
			break;
		case listen:
			if(b3){
				task2.active = 0x01; 
				PORTB = 0xFF;
				t = 0;
				break;
			}
			else{
				break;
			}
		default:
			break;
	}
	return w_state;
}

void updateCount(int incValue) {
	const char MAX_COUNT = 9;
	const char MIN_COUNT = 0;
	unsigned char newCount = count + incValue;
	if (MIN_COUNT <= newCount && newCount <= MAX_COUNT) {
		count = newCount;
	}
}

// Implement scheduler code from PES.
int main()
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	LCD_init();
	LCD_ClearScreen();
	
	//LCD_DisplayString(1, "Hello World!");
	
	PORTB = 0xFF;

	// Period for the tasks
	unsigned long int SMTick1_calc = 50;
	unsigned long int SMTick2_calc = 1000;
	unsigned long int SMTick3_calc = 50;

	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(tmpGCD, SMTick1_calc);
	tmpGCD = findGCD(tmpGCD, SMTick2_calc);
	tmpGCD = findGCD(tmpGCD, SMTick3_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD;

	//Declare an array of tasks
	//static task task1, task2;
	task *tasks[] = {&task1, &task2, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.active = 0x01;
	task1.TickFct = &IncDec_tick;//Function pointer for the tick.
	
	// Task 2
	task2.state = -1;
	task2.period = SMTick2_period;
	task2.elapsedTime = SMTick2_period;
	task2.active = 0x01;
	task2.TickFct = &LCD_tick;
	
	// Task 3
	task3.state = -1;
	task3.period = SMTick3_period;
	task3.elapsedTime = SMTick3_period;
	task3.active = 0x01;
	task3.TickFct = &Wake_tick;

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	
	PORTB = 0x01;

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

