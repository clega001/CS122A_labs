/*
 * two_test.c
 *
 * Created: 11/19/2019 7:53:16 PM
 * Author : Christian Legaspino
 */ 

//Includes
#include "bit.h"
#include <avr/io.h>
#include "usart_ATmega1284.h"
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/delay.h>


//Global Variables
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
const unsigned long PERIOD = 100; //Preset Period

unsigned char r = 0x00;

//Functions
void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;
	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}


typedef struct Task {
	int state; // Task’s current state
	unsigned long period; // Task period
	unsigned long elapsedTime; // Time elapsed since last task tick
	int (*TickFct)(int); // Task tick function
} task;

const unsigned char tasksSize = 2;
task tasks[2];

void TimerISR()
{
	unsigned char i;
	for (i = 0;i < tasksSize;++i)
	{
		if ((tasks[i].elapsedTime >= tasks[i].period))
		{
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += PERIOD;
	}
}

ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char color = 0x00;
unsigned char limit = 10;
unsigned char cnt = 0;
unsigned char ready = 0x00;

//State Machines
enum blinkState{init, on, off} state;
int blink(int state){
	switch(state){
		case init:
			state = on; break;
		case on:
			state = off;
			break;
		case off:
			state = on;
			break;
		default:
			state = init; break;
	}
	switch(state){
		case init:
			break;
		case on:
			if(color == 0x02){
				PORTA = 0x02;
			} else {
				PORTA = 0x01;
			}
			break;
		case off:
			if(color == 0x02){
				if(cnt >= 5){
					TimerOff();
					tasks[1].period = 1000;
					color = 0x01;
					TimerOn();
				} else {
					PORTA = 0x02;
					cnt += 1;
				}
			} else {
				PORTA = 0x00;
			}
			break;
		default:
			break;
	}
	return state;
}

enum readState{start, action} s;
int readUSART(int s){
	switch(s){
		case start:
			s = action; break;
		case action:
			s = action; break;
		default:
			s = start; break;
	}
	switch(s){
		case start:
			break;
		case action:
			if(GetBit(r,2)){
				TimerOff();
				tasks[1].period = 10;
				color = 0x02;
				TimerSet(PERIOD);
				TimerOn();
				break;
			}
			else if(GetBit(r,1)){
				TimerOff();
				tasks[1].period = 100;
				TimerSet(PERIOD);
				TimerOn();
				break;			
			}
			else if(GetBit(r,0)){
				TimerOff();
				tasks[1].period = 500;
				TimerSet(PERIOD);
				TimerOn();
				break;
			}
			else{
				break;
			}
		default:
			break;
	}
	return s;
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0x00; PORTD = 0xFF;

	initUSART(0);

	tasks[0].state = start;
	tasks[0].period = 100;
	tasks[0].elapsedTime = 0;
	tasks[0].TickFct = &readUSART;

	tasks[1].state = init;
	tasks[1].period = 1000;
	tasks[1].elapsedTime = 0;
	tasks[1].TickFct = &blink;
	
	TimerSet(PERIOD);
	TimerOn();
	while (1)
	{
		if(USART_HasReceived(0))
		{
			r = USART_Receive(0);
			USART_Flush(0);
		}
	}
}