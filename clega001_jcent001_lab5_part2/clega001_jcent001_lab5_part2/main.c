/*
 * Christian Legaspino clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 5 Exercise 2
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
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

void transmit_data(unsigned char data){
	for(unsigned int i = 0; i < 8; i++){
		PORTC = SetBit(PORTC, 7, 1); //Set SRCLR to high
		PORTC = SetBit(PORTC, 6, 0); //Set SRCLK to low
		PORTC = SetBit(PORTC, 4, GetBit(data,i)); //Set SER to send bit
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
unsigned char tmp = 0x01;
unsigned char tmp1 = 0x00;
unsigned char tmp2 = 0x01;
unsigned char tmp3 = 0x00;
unsigned char go = 0x01;
unsigned char dir = 0x01;

#define b1 (~PINB & 0x01)
#define b2 (~PINB & 0x02)
//--------End Shared/Global Variables------------------------------------------------
//--------User defined FSMs---------------------------------------------------
enum DisplayState {init, wait, inc_p, inc_r, dec_p, dec_r, reset_p, reset_r, off, off_p, off_r} d_state;
int d_Tick(){
	//Transitions
	switch(d_state){
		case init:
			d_state = wait;
			break;
		case wait:
			if(b1 && !b2){
				d_state = inc_p;
				break;
			}
			else if(!b1 && b2){
				d_state = dec_p;
				break;
			}
			else if(b1 && b2){
				d_state = reset_p;
				break;
			}
			else{
				d_state = wait;
				break;
			}
		case inc_p:
			if(b1 && !b2){
				d_state = inc_p;
				break;
			}
			else if(b1 && b2){
				d_state = reset_p;
				break;
			}
			else{
				d_state = inc_r;
				break;
			}
		case inc_r:
			d_state = wait;
			break;
		case dec_p:
			if(!b1 && b2){
				d_state = dec_p;
				break;
			}
			else if(b1 && b2){
				d_state = reset_p;
				break;
			}
			else{
				d_state = dec_r;
				break;
			}
		case dec_r:
			d_state = wait;
			break;
		case reset_p:
			if(b1 && b2){
				d_state = reset_p;
				break;
			}
			else{
				d_state = reset_r;
				break;
			}
		case reset_r:
			d_state = off;
			break;
		case off:
			if(b1 && b2){
				d_state = off_p;
				break;
			}
			else{
				d_state = off;
				break;
			}
		case off_p:
			if(b1 && b2){
				d_state = off_p;
				break;
			}
			else{
				d_state = off_r;
				break;
			}
		case off_r:
			d_state = wait;
			break;
		default:
			d_state = init;
			break;
	}
	//Actions
	switch(d_state){
		case init:
			break;
		case wait:
			//transmit_data(go);
			break;
		case inc_p:
			break;
		case inc_r:
			if(go != 3){go++;}
			break;
		case dec_p:
			break;
		case dec_r:
			if(go != 1){go--;}
			break;
		case reset_p:
			break;
		case reset_r:
			tmp = go;
			go = 0;
			break;
		case off:
			transmit_data(go);
			break;
		case off_p:
			break;
		case off_r:
			go = tmp;
			tmp = 0;
			break;
		default:
			break;
		
	}
	return d_state;
}
////////////////////////////////////////////////////////////////////////
enum pat1States{init1, wait1, l1, l2, l3, l4} p1_state;
int p1Tick(){
	//Transition
	switch(p1_state){
		case init1:
			p1_state = wait1;
			break;
		case wait1:
			if(go == 0x01){
				p1_state = l1;
				dir = 0x01;
				break;
			}
			else{
				p1_state = wait1;
				break;
			}
		case l1:
			if(go == 0x01){
				p1_state = l2;
				break;
			}
			else{
				p1_state = wait1;
				break;
			}
		case l2:
			if(go == 0x01 && dir == 0x01){
				p1_state = l3;
				break;
			}
			else if(go == 0x01 && dir == 0x00){
				p1_state = l1;
				break;
			}
			else{
				p1_state = wait1;
				break;
			}
		case l3:
			if(go == 0x01 && dir == 0x01){
				p1_state = l4;
				break;
			}
			else if(go == 0x01 && dir == 0x00){
				p1_state = l2;
				break;
			}
			else{
				p1_state = wait1;
				break;
			}
		case l4:
			if(go == 0x01){
				p1_state = l3;
				break;
			}
			else{
				p1_state = wait1;
			}
		default:
			p1_state = init1;
			break;
	}
	//Action
	switch(p1_state){
		case init1:
			break;
		case wait1:
			break;
		case l1:
			tmp1 = 0x81;
			transmit_data(tmp1);
			dir = 0x01;
			break;
		case l2:
			tmp1 = 0xC3;
			transmit_data(tmp1);
			break;
		case l3:
			tmp1 = 0xE7;
			transmit_data(tmp1);
			break;
		case l4:
			tmp1 = 0xFF;
			transmit_data(tmp1);
			dir = 0x00;
			break;
		default:
			break;
	}
	return p1_state;
}
///////////////////////////////////////////////////////////////////
enum pat2States{init2, wait2, k1, k2} p2_state;
int p2Tick(){
	//Transition
	switch(p2_state){
		case init2:
			p2_state = wait2;
			break;
		case wait2:
			if(go == 0x02){
				p2_state = k1;
				break;
			}
			else{
				p2_state = wait2;
				break;
			}
		case k1:
			if(go == 0x02){
				if(tmp2 != 0x80){
					p2_state = k1;
					break;
				}
				else{
					p2_state = k2;
					break;
				}
			}
			else{
				p2_state = wait2;
				break;
			}
		case k2:
			if(go == 0x02){
				if(tmp2 != 0x01){
					p2_state = k2;
					break;
				}
				else{
					p2_state = k1;
					break;
				}
			}
			else{
				p2_state = wait2;
				break;
			}
		default:
			p2_state = init2;
			break;
	}	
	//Action
	switch(p2_state){
		case init2:
			break;
		case wait2:
			break;
		case k1:
			tmp2 = tmp2 << 1;
			transmit_data(tmp2);
			break;
		case k2:
			tmp2 = tmp2 >> 1;
			transmit_data(tmp2);
			break;
		default:
			break;
	}
	return p2_state;
}
//////////////////////////////////////////////////////////////////////////
enum pat3States{init3, wait3, m1, m2}p3_state;
int p3Tick(){
	//Transition
	switch(p3_state){
		case init3:
			p3_state = wait3;
			break;
		case wait3:
			if(go == 0x03){
				p3_state = m1;
				break;
			}
			else{
				p3_state = wait3;
				break;
			}
		case m1:
			if(go == 0x03){
				p3_state = m2;
				break;
			}
			else{
				p3_state = wait3;
				break;
			}
		case m2:
			if(go == 0x03){
				p3_state = m1;
				break;
			}
			else{
				p3_state = wait3;
				break;
			}
		default:
			p3_state = init3;
			break;
	}	
	//Action
	switch(p3_state){
		case init3:
			break;
		case wait3:
			break;
		case m1:
			tmp3 = 0xAA;
			transmit_data(tmp3);
			break;
		case m2:
			tmp3 = 0x55;
			transmit_data(tmp3);
			break;
		default:
			break;
	}
	return p3_state;
}

// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;

	// Period for the tasks
	unsigned long int SMTick1_calc = 50;
	unsigned long int SMTick2_calc = 100;
	unsigned long int SMTick3_calc = 100;
	unsigned long int SMTick4_calc = 100;

	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(tmpGCD, SMTick1_calc);
	tmpGCD = findGCD(tmpGCD, SMTick2_calc);
	tmpGCD = findGCD(tmpGCD, SMTick3_calc);
	tmpGCD = findGCD(tmpGCD, SMTick4_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	unsigned long int SMTick3_period = SMTick3_calc/GCD;
	unsigned long int SMTick4_period = SMTick4_calc/GCD;

	//Declare an array of tasks
	static task task1, task2, task3, task4;
	task *tasks[] = {&task1, &task2, &task3, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &d_Tick;//Function pointer for the tick.
	
	// Task 2
	task2.state = -1;
	task2.period = SMTick2_period;
	task2.elapsedTime = SMTick2_period;
	task2.TickFct = &p1Tick;
	
	// Task 3
	task3.state = -1;
	task3.period = SMTick3_period;
	task3.elapsedTime = SMTick3_period;
	task3.TickFct = &p2Tick;
	
	// Task 4
	task4.state = -1;
	task4.period = SMTick4_period;
	task4.elapsedTime = SMTick4_period;
	task4.TickFct = &p3Tick;

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();

	PORTC = SetBit(PORTC, 7, 1); //Set SRCLR to high
	PORTC = SetBit(PORTC, 5, 0); //Set RCLK to low

	unsigned char a = 0xAA;
	transmit_data(a);

	unsigned short i;
	while(1) {
		for ( i = 0; i < numTasks; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
			PORTC = SetBit(PORTC, 5, 0); //Set RCLK to low
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}
