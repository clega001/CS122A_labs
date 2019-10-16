#include <avr/io.h>
#include "usart_ATmega1284.h"
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>


volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
const unsigned long p = 50;



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

//--------Shared Variables----------------------------------------------------
unsigned char x = 0x00;
unsigned char shifty = 0x80;
unsigned char bouncey = 0xC0;
unsigned char pattern = 0x00;
unsigned char speed = 0x00;
unsigned char tempo = 50;
unsigned char cnt = 0x00;
//--------End Shared Variables------------------------------------------------

//Servant
void SPI_SlaveInit(void){
	/* Set MISO output, all others input */

	/*DDR_SPI = (1<<DD_MISO);*/
	//PORTB = (1<<PORTB4)|(1<<PORTB6);
	//PORTB = (1<<PORTB6);
	DDRB = (1<<DDRB4)|(1<<DDRB6);
	
	/* Enable SPI */
	SPCR = (1<<SPE);
}

char SPI_SlaveReceive(void){
	/* Wait for reception complete */
	while(!(SPSR & (1<<SPIF)))
	;
	/* Return Data Register */
	return SPDR;
}

//Slave Receive
//--------User defined FSMs---------------------------------------------------
enum Receive_States{r_start, Wait} r_state;
int TickFct_Receive(int r_state){
	switch(r_state){
		case r_start:
		r_state = Wait; break;
		case Wait:
		r_state = Wait; break;
		default:
		r_state = Wait; break;
	}
	switch(r_state){
		case r_start:
		break;
		case Wait:
		x = SPI_SlaveReceive();
		
		pattern = x & 0xF0;
		speed = x & 0x0F;
		
		default:
		break;
	}
	return r_state;
}

enum Speed_State{s_start, pick} s_state;
int TickFct_Speed(){
	switch(s_state)	{
		case s_start:
		s_state = pick; break;
		case pick:
		s_state = pick; break;
		default:
		s_state = s_start; break;
	}
	switch(s_state){
		case s_start:
		break;
		case pick:
		break;
	}
	return s_state;
}


enum Pattern1_States{p1_start, p1_wait, left, right} p1_state;
int TickFct_P1(){
	switch(p1_state){//Transition
		case p1_start:
			p1_state = p1_wait; break;
		case p1_wait:
			if(pattern == 0xA0){
				p1_state = left; break;
				}else{p1_state = p1_wait; break;}
		case left:
			if(pattern == 0xA0){
				p1_state = right; break;
			}else{p1_state = p1_wait; break;}
		case right:
			if(pattern == 0xA0){
				p1_state = left; break;
			}else{p1_state = p1_wait; break;}
		default:
			p1_state = p1_start; break;
	}
		switch(p1_state){//Action
			case p1_start:
			break;
			case p1_wait:
			break;
			case left:
			PORTC = 0xF0; break;
			case right:
			PORTC = 0x0F; break;
			default:
			break;
		}
		return p1_state;
	}

enum Pattern2_States {p2_start, p2_wait, on, off} p2_state;
int TickFct_P2(){
	switch(p2_state){
		case p2_start:
			p2_state = p2_wait; break;
		case p2_wait:
			if(pattern == 0xB0){
				p2_state = on; break;
			}else{p2_state = p2_wait; break;}
		case on:
			if(pattern == 0xB0){
				p2_state = off; break;
			}else{p2_state = p2_wait; break;}
		case off:
			if(pattern == 0xB0){
				p2_state = on; break;
			}else{p2_state = p2_wait; break;}
		default:
			p2_state = p2_start; break;
		}
	switch(p2_state){
		case p2_start:
			break;
		case p2_wait:
			break;
		case on:
			PORTC = 0xAA; break;
		case off:
			PORTC = 0x55; break;
		default:
			break;
	}
	return p2_state;
}

enum Pattern3_States{p3_start, p3_wait, shift} p3_state;
int TickFct_P3(){
	switch(p3_state){
		case p3_start:
			p3_state = p3_wait; break;
		case p3_wait:
			if(pattern == 0xC0){
				p3_state = shift; break;
			}else{p3_state = p3_wait; break;}
		case shift:
			if(pattern == 0xC0){
				p3_state = shift; break;
			}else{p3_state = p3_wait; break;}
		default:
			p3_state = p3_start; break;
	}
	switch(p3_state){
		case p3_start:
			break;
		case p3_wait:
			break;
		case shift:
			PORTC = shifty;
			if(shifty == 0x01){
				shifty = 0x80;
			}else{shifty = shifty >> 1;}
			break;
		default:
			break;
	}
	return p3_state;
}

enum Pattern4_States{p4_start, p4_wait, bounce_right, bounce_left} p4_state;
int TickFct_P4(){
	switch(p4_state){
		case p4_start:
			p4_state = p4_wait; break;
		case p4_wait:
			if(pattern == 0xD0){
			p4_state = bounce_right; break;
			}else{p4_state = p4_wait; break;}
		case bounce_right:
			if(pattern == 0xD0 && bouncey != 0x03){
				p4_state = bounce_right; break;
			}
			else if(pattern == 0xD0 && bouncey == 0x03){
				p4_state = bounce_left; break;
			}
			else{
				p4_state = p4_wait; break;
			}
		case bounce_left:
			if(pattern == 0xD0 && bouncey != 0xC0){
				p4_state = bounce_left; break;
			}
			else if(pattern == 0xD0 && bouncey == 0xC0){
				p4_state = bounce_right; break;
			}
			else{
				p4_state = p4_wait; break;
			}
		default:
			p4_state = p4_start; break;
	}
	switch(p4_state){
		case p4_start:
			break;
		case p4_wait:
			break;
		case bounce_right:
			PORTC = bouncey;
			bouncey = bouncey >> 1;
			break;
		case bounce_left:
			PORTC = bouncey;
			bouncey = bouncey << 1;
			break;
		default:
			break;
	}
	return p4_state;
}
// --------END User defined FSMs-----------------------------------------------

void TaskRedifine(unsigned char tasknum, unsigned char setPeriod, int (*stateMachine)(int))
{
	TimerOff();
	tasks[tasknum].period = setPeriod;
	tasks[tasknum].state = 0;
	tasks[tasknum].TickFct = &stateMachine;
	TimerOn();
}


unsigned char m = 0x00;
unsigned char n = 0x00;

int main(void)
{
	// Set Data Direction Registers
	DDRB = 0x00; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;

	unsigned char k = 0x00;

	tasks[0].state = r_start;
	tasks[0].period = 50;
	tasks[0].elapsedTime = 0;
	tasks[0].TickFct = &TickFct_Receive;
	
	tasks[1].state = s_start;
	tasks[1].period = 50;
	tasks[1].elapsedTime = 0;
	tasks[1].TickFct = &TickFct_Speed;
	
	tasks[0].state = p1_start;
	tasks[0].period = 50;
	tasks[0].elapsedTime = 0;
	tasks[0].TickFct = &TickFct_P1;
		
		
	TimerSet(50);
	TimerOn();
	while (1)
	{
		
		if(pattern == 0xA0){
			m = TickFct_P1;
		}
		else if(pattern == 0xB0){
			m = TickFct_P2;
		}
		else if(pattern == 0xC0){
			m = TickFct_P3;
		}
		else if(pattern == 0xD0){
			m = TickFct_P4;
		}
		
		if(speed == 0x01){
			n = 2000;
		}
		else if(speed == 0x02){
			n = 1000;
		}
		else if(speed == 0x03){
			n = 500;
		}
		else if(speed == 0x04){
			n = 250;
		}
		else if(speed == 0x05){
			n = 100;
		}
		else if(speed == 0x06){
			n = 50;
		}
		
		TaskRedifine(2, n, m);
		
		continue;
	}
}

