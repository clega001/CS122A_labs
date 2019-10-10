/*
 * lab2_part3_alt.c
 *
 * Created: 10/10/2019 1:42:40 AM
 * Author : Christian Legaspino
 */ 

// #include <avr/io.h>
// #include "usart_ATmega1284.h"
// #include <avr/interrupt.h>
// #include <stdlib.h>
// #include <stdio.h>
// 
// 
// 
// volatile unsigned char TimerFlag = 0; 
// unsigned long _avr_timer_M = 1;
// unsigned long _avr_timer_cntcurr = 0; 
// const unsigned long p = 100; 
// unsigned char cnt = 0x00;
// 
// unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
// {
// 	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
// }
// 
// unsigned long int findGCD(unsigned long int a, unsigned long int b)
// {
// 	unsigned long int c;
// 	while(1){
// 		c = a%b;
// 		if(c==0){return b;}
// 		a = b;
// 		b = c;
// 	}
// 	return 0;
// }
// 
// void TimerOn() {
// 	// AVR timer/counter controller register TCCR1
// 	TCCR1B = 0x0B;
// 	// AVR output compare register OCR1A.
// 	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
// 	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt
// 
// 	//Initialize avr counter
// 	TCNT1=0;
// 
// 	_avr_timer_cntcurr = _avr_timer_M;
// 	SREG |= 0x80; // 0x80: 1000000
// }
// 
// void TimerOff() {
// 	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
// }
// typedef struct Task {
// 	int state; // Task’s current state
// 	unsigned long period; // Task period
// 	unsigned long elapsedTime; // Time elapsed since last task tick
// 	int (*TickFct)(int); // Task tick function
// } task;
// 
// const unsigned char tasksSize = 1;
// task tasks[1];
// 
// void TimerISR()
// {
// 	unsigned char i;
// 	for (i = 0;i < tasksSize;++i)
// 	{
// 		if ((tasks[i].elapsedTime >= tasks[i].period))
// 		{
// 			tasks[i].state = tasks[i].TickFct(tasks[i].state);
// 			tasks[i].elapsedTime = 0;
// 		}
// 		tasks[i].elapsedTime += p;
// 	}
// }
// ISR(TIMER1_COMPA_vect) {
// 	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
// 	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
// 	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
// 		TimerISR(); // Call the ISR that the user uses
// 		_avr_timer_cntcurr = _avr_timer_M;
// 	}
// }
// void TimerSet(unsigned long M) {
// 	_avr_timer_M = M;
// 	_avr_timer_cntcurr = _avr_timer_M;
// }
// 
// enum master {m_start, on, off};
// int Master(int state){
// 	switch(state){
// 		case m_start:
// 			state = on;
// 			break;
// 		case on:
// 			state = off;
// 			break;
// 		case off:
// 			state = on;
// 			break;
// 		default:
// 			state = m_start;
// 			break;
// 	}
// 	switch(state){
// 		case m_start:
// 			break;
// 		case on:
// 			PORTA = 0x01;
// 			PORTC = 0x01;
// 			break;
// 		case off:
// 			PORTA = 0x00;
// 			PORTC = 0x01;
// 			break;
// 		default:
// 			PORTA = PORTA;
// 			break;
// 	}
// 	USART_Send(PORTA, 1);
// 	return state;
// }
// 
// enum follower {f_start, wait};
// int Follower(int state){
// 	switch(state){
// 		case f_start:
// 			state = wait; break;
// 		case wait:
// 			state = wait; break;
// 		default:
// 			state = f_start; break;
// 	}
// 	switch(state){
// 		case f_start:
// 			break;
// 		case wait:
// 			PORTC = 0x00;
// 			if(USART_HasReceived(0)){
// 				PORTA = USART_Receive(0);
// 				USART_Flush(0);
// 				cnt = 0x00;
// 			}else{
// 				cnt += 1;
// 			}
// 	}
// 	return state;
// }
// 
// unsigned char s = 0x00;
// 
// int main(void)
// {
// 	DDRA = 0xFF; PORTA = 0x00;
// 	DDRB = 0x00; PORTB = 0xFF;
// 	DDRC = 0xFF; PORTC = 0x00;
// 
// 	initUSART(0);
// 	initUSART(1);
// 
// // 	if(s){
// // 		tasks[0].state = m_start;
// // 		tasks[0].period = 1000;
// // 		tasks[0].elapsedTime = 0;
// // 		tasks[0].TickFct = &Master;
// // 	}else{
// // 		tasks[0].state = f_start;
// // 		tasks[0].period = 100;
// // 		tasks[0].elapsedTime = 0;
// // 		tasks[0].TickFct = &Follower;
// // 	}
// 	
// 		tasks[0].state = f_start;
// 		tasks[0].period = 100;
// 		tasks[0].elapsedTime = 0;
// 		tasks[0].TickFct = &Follower;
// 	
// 	TimerSet(p);
// 	TimerOn();
// 	
// 	
// 	while (1)
// 	{
// 		if(cnt > 30)
// 		{
// 			TimerOff();
// 			s = 0x01;
// 			tasks[0].state = m_start;
// 			tasks[0].period = 1000;
// 			tasks[0].elapsedTime = 0;
// 			tasks[0].TickFct = &Master;
// 			TimerSet(p);
// 			TimerOn();
// 			USART_Send(PORTA, 1);
// 			cnt = 0x00;
// 		}
// 		continue;
// 	}
// }

//Includes
#include <avr/io.h>
#include "usart_ATmega1284.h"
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>


//Global Variables
unsigned char Master_Servant; //1 if master, 0 is servant
unsigned char count; //Counter
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
const unsigned long PERIOD = 100; //Preset Period
#define B0 (~PINB & 0x01)

unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value)
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
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
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum follower {f_start, wait};
int Follower(int state){
// 	switch(state){
// 		case f_start:
// 		state = wait; break;
// 		case wait:
// 		state = wait; break;
// 		default:
// 		state = f_start; break;
// 	}
// 	switch(state){
// 		case f_start:
// 		break;
// 		case wait:
	if(USART_HasReceived(0)){
		PORTA = USART_Receive(0);
		USART_Flush(0);
		count = 0x00;
		}else{
		count++;
	}
/*		}*/
	return state;
}

enum master {Start, BSS_Low, BSS_High};
int Master(int state){
	switch(state){
		case Start:
			state = BSS_Low;
			break;
		case BSS_Low:
			state = BSS_High;
			break;
		case BSS_High:
			state = BSS_Low;
			break;
		default:
			state = Start;
			break;
	}	
	switch(state){
		case BSS_Low:
			PORTA = 0x00;
			count = 0;
			
			if(USART_HasReceived(0)){
				TimerOff();
	 			Master_Servant = 0x00;
	 			tasks[0].state = Start;
	 			tasks[0].period = 100;
	 			tasks[0].elapsedTime = 0;
	 			tasks[0].TickFct = &Follower;
	 			TimerSet(PERIOD);
	 			TimerOn();
			}
			
			break;
		case BSS_High:
			PORTA = 0x01;
			count = 0;
			
			if(USART_HasReceived(0)){
				TimerOff();
				Master_Servant = 0x00;
				tasks[0].state = Start;
				tasks[0].period = 100;
				tasks[0].elapsedTime = 0;
				tasks[0].TickFct = &Follower;
				TimerSet(PERIOD);
				TimerOn();
			}
			
			
			break;
		default:
			PORTA = 0x00;
			break;
	}
	USART_Send(PORTA, 1);
	return state;
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;

	Master_Servant = 0x00;
	initUSART(0);
	initUSART(1);

	tasks[0].state = Start;
	tasks[0].period = 100;
	tasks[0].elapsedTime = 0;
	tasks[0].TickFct = &Follower;
	
	TimerSet(PERIOD);
	TimerOn();
	while (1)
	{
		PORTC = Master_Servant;
		if(count > 30 && !Master_Servant)
		{
			TimerOff();
			Master_Servant = 0x01;
			tasks[0].state = Start;
			tasks[0].period = 1000;
			tasks[0].elapsedTime = 0;
			tasks[0].TickFct = &Master;
			TimerSet(PERIOD);
			TimerOn();

		}
		continue;
	}
}