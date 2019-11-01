/*
 * Christian Legaspino clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 5 Exercise 5
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 */
////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/interrupt.h>
#include <bit.h>
#include <timer.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdbool.h>
#include <alloca.h>
#include <stdlib.h>

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
unsigned char GetBitShort(unsigned short x, unsigned short k) {
	return ((x & (0x01 << k)) != 0);
}
void transmit_dataShort(unsigned short data){
	for(unsigned int i = 0; i < 16; i++){
		PORTC = SetBit(PORTC, 7, 1); //Set SRCLR to high
		PORTC = SetBit(PORTC, 6, 0); //Set SRCLK to low
		PORTC = SetBit(PORTC, 4, GetBitShort(data,i)); //Set SER to send bit
		PORTC = SetBit(PORTC, 6, 1); //Set SRCLK to high
	}
	PORTC = SetBit(PORTC, 5, 1);
	PORTC = SetBit(PORTC, 7, 0); //Set SRCLR to low
}
//--------Task scheduler data structure---------------------------------------
typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;
//--------End Task scheduler data structure-----------------------------------
//--------Shared/Global Variables----------------------------------------------------
unsigned short tmp = 0x0001;
unsigned short c = 0x2000;
unsigned short final = 0x0000;
unsigned short test = 0xFFFF;
unsigned short zero = 0x0000;
unsigned char dir = 0x01;
#define b1 (~PINB & 0x01) //go left ... << 1
#define b2 (~PINB & 0x02) //jump 
#define b3 (~PINB & 0x04) //go right ... >> 1
//--------End Shared/Global Variables------------------------------------------------
//--------User defined FSMs---------------------------------------------------
enum PlayerState {pinit, wait, inc_p, inc_r, dec_p, dec_r, jump_p, jump_r} pstate;
int PlayerTick(){
	//Transitions
	switch(pstate){
		case pinit:
			pstate = wait;
			break;
		case wait:
			if(b1 && !b2 && !b3){
				pstate = dec_p;
				break;
			}
			else if(!b1 && b2 && !b3){
				pstate = jump_p;
				break;
			}
			else if(!b1 && !b2 && b3){
				pstate = inc_p;
				break;
			}
			else{
				pstate = wait;
				break;
			}
		case inc_p:
			if(b3){
				pstate = inc_p;
				break;
			}
			else{
				pstate = inc_r;
				break;
			}
		case inc_r:
			pstate = wait;
			break;
		case dec_p:
			if(b1){
				pstate = dec_p;
				break;
			}
			else{
				pstate = dec_r;
				break;
			}
		case dec_r:
			pstate = wait;
			break;
		case jump_p:
			if(b2){
				pstate = jump_p;
				break;
			}
			else{
				pstate = jump_r;
				break;
			}
		case jump_r:
			pstate = wait;
			break;
		default:
			pstate = pinit;
			break;
	}
	//Actions
	switch(pstate){
		case pinit:
			break;
		case wait:
			break;
		case inc_p:
			break;
		case inc_r:
			if(c != 0x0001){c = c >> 1;}
			dir = 0x01;
			break;
		case dec_p:
			break;
		case dec_r:
			if(c != 0x8000){c = c << 1;}
			dir = 0x00;
			break;
		case jump_p:
			break;
		case jump_r:
			if(((c == 0x0008) || (c == 0x0004) || (c == 0x0002) || (c == 0x0001)) && dir == 0x01){
				c = 0x0001; break;
			}
			else if(dir == 0x01){
				c = c >> 4; break;
			}
			else if(((c == 0x8000) || (c == 0x4000) || (c == 0x2000) || (c == 0x1000)) && dir == 0x00){
				c = 0x8000; break;
			}
			else if(dir == 0x00){
				c = c << 4; break;
			}
		default:
			break;
		
	}
	return pstate;
}

//Set Enemy LED
enum EnemyState {einit, up, down} estate;
int EnemyTick(){
	//Transitions
	switch(estate){
		case einit:
			estate = up;
			break;
		case up:
			if(tmp != 0x8000){
				estate = up; break;
			}else{
				estate = down; break;
			}
		case down:
			if(tmp != 0x0001){
				estate = down; break;
			}else{
				estate = up; break;
			}
		default:
			estate = einit;
			break;
	}
	//Actions
	switch(estate){
		case einit:
			break;
		case up:
			tmp = tmp << 1;
			//transmit_dataShort(tmp);
			break;
		case down:
			tmp = tmp >> 1;
			//transmit_dataShort(tmp);
		default:
			break;
		
	}
	return estate;
}


// Display LED
enum LEDState{LEDinit, led, on1, off1, on2, off2} led_state;
int LEDTick(){
	switch(led_state){
		case LEDinit:
			led_state = led; break;
		case led:
			if(tmp == c){
				led_state = on1; break;
			}else{
				led_state = led_state; break;
			}
		case on1:
			led_state = off1; break;
		case off1:
			led_state = on2; break;
		case on2:
			led_state = off2; break;
		case off2:
			led_state = led; break;
		default:
			led_state = LEDinit; break;
	}
	switch(led_state){
		case LEDinit:
			break;
		case led:
			final = tmp | c;
			transmit_dataShort(final);
			break;
		case on1:
			transmit_dataShort(test);
			break;
		case off1:
			transmit_dataShort(zero);
			break;
		case on2:
			transmit_dataShort(test);
			break;
		case off2:
			transmit_dataShort(zero);
			c = 0x8000;
			tmp = 0x0001;
			dir = 0x01;
		default:
			break;
	}
	return led_state;
}

// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	// Period for the tasks
	unsigned long int SMTick1_calc = 50;
	unsigned long int SMTick2_calc = 500;
	unsigned long int SMTick3_calc = 100;

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
	static task task1, task2, task3;
	task *tasks[] = {&task1, &task2, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &PlayerTick;//Function pointer for the tick.
	
	// Task 2
	task2.state = -1;
	task2.period = SMTick2_period;
	task2.elapsedTime = SMTick2_period;
	task2.TickFct = &EnemyTick;
	
	// Task 3
	task3.state = -1;
	task3.period = SMTick3_period;
	task3.elapsedTime = SMTick3_period;
	task3.TickFct = &LEDTick;

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();

	PORTC = SetBit(PORTC, 7, 1); //Set SRCLR to high
	PORTC = SetBit(PORTC, 5, 0); //Set RCLK to low
	
	unsigned short i;
	while(1) {
		for ( i = 0; i < numTasks; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
			//transmit_dataShort(test);
			PORTC = SetBit(PORTC, 5, 0); //Set RCLK to low
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}

