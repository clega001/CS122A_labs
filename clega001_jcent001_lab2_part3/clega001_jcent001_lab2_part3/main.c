/*
 * Christian Legaspino (clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 2 Exercise 3
 * Lab Description: 
 * 
 * I acknowledge all content contained herein, excluding template 
 *		or example code, is my own original work.
 */ 

#include <avr/io.h>
#include "usart_ATmega1284.h"
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>

 
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
const unsigned long p = 100;


unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

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

const unsigned char tasksSize = 1;
task tasks[1];

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
		tasks[i].elapsedTime += p;
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
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char cnt = 0x00;
unsigned char s = 0x00;
unsigned long p = 100;


enum follower {f_start, wait};
int Follower(int state){
	switch(state){
		case f_start:
			state = wait; break;
		case wait:
			state = wait; break;
		default:
			state = f_start; break;
	}
	switch(state){
		case f_start:
			break;
		case wait:
			if(USART_HasReceived(0)){
				PORTA = USART_Receive(0);
				USART_Flush(0);
				cnt = 0x00;
			}else{cnt++;}
	}
	return state;
}

enum master {m_start, on, off};
int Master(int state){
	switch(state){
		case m_start:
			state = on;
			break;
		case on:
			state = off;
			break;
		case off:
			state = on;
			break;
		default:
			state = m_start;
			break;
	}
	switch(state){
		case off:
			PORTA = 0x00;
			USART_Send(PORTA, 1);
			cnt = 0;
			if(USART_HasReceived(0)){
				TimerOff();
				s = 0x00;
				tasks[0].state = f_start;
				tasks[0].period = 100;
				tasks[0].elapsedTime = 0;
				tasks[0].TickFct = &Follower;
				TimerSet(p);
				TimerOn();
			}
			break;
		case on:
			PORTA = 0x01;
			USART_Send(PORTA, 1);
			cnt = 0;
			if(USART_HasReceived(0)){
				TimerOff();
				s = 0x00;
				tasks[0].state = f_start;
				tasks[0].period = 100;
				tasks[0].elapsedTime = 0;
				tasks[0].TickFct = &Follower;
				TimerSet(p);
				TimerOn();
			}
			break;
		default:
			PORTA = 0x00;
			break;
	}
	return state;
}


int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;

	initUSART(0);
	initUSART(1);


	s = 0x00;
	tasks[0].state = f_start;
	tasks[0].period = 100;
	tasks[0].elapsedTime = 0;
	tasks[0].TickFct = &Follower;
	
	
	unsigned char tmp = findGCD(1000,100);
	
	TimerSet(tmp);
	TimerOn();
	while (1)
	{
		PORTC = s;
		if(cnt > 30 && !s)
		{
			TimerOff();
			s = 0x01;
			tasks[0].state = m_start;
			tasks[0].period = 1000;
			tasks[0].elapsedTime = 0;
			tasks[0].TickFct = &Master;
			TimerSet(p);
			TimerOn();

		}
		continue;
	}
}