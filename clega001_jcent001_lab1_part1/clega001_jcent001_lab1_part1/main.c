/*
 * Christian Legaspino (clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 1 Exercise 1
 * Lab Description: Three LEDs blink sequentially at 500ms. A single LED blinks for every 1000ms. A button is added, once pressed, the three blinking
 *					LEDs are paused.
 * 
 * I acknowledge all content contained herein, excluding template 
 *		or example code, is my own original work.
 */ 
/////////////////////////////////////////////////////////
#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum BL_States{BL_Start, BL_Blink} BL_state;
enum TL_States{TL_Start, TL_Blink, TL_Blink_Again} TL_state;
enum Butt_States{Butt_Start, Butt_Press, Butt_Hold, hold, Butt_press, Butt_hold} Butt_state;
enum Combine_States{Com_Start, Com_Action} Com_states;

unsigned char blinkingLED = 0x10;
unsigned char threeLED = 0x04;
unsigned char hold1 = 0x00;
unsigned char hold2 = 0x00;
unsigned char b = 0x00;

void TickFct_BL(){
	
	/*blinkingLED = PORTB & 0x10;*/
	
	switch(BL_state){
		case BL_Start:
			BL_state = BL_Blink;
			break;
		case BL_Blink:
			BL_state = BL_Start;;
			break;
		default:
			BL_state = BL_Start;
			break;
	}
	switch(BL_state){
		case BL_Start:
			blinkingLED = 0x01;
			hold1 = blinkingLED;
			break;
		case BL_Blink:
			blinkingLED = 0x00;
			hold1 = blinkingLED;
			break;
		default:
			break;
	}
}
void TickFct_TL(){
	
	/*threeLED = PORTB & 0x07;*/
	
	switch(TL_state){
		case TL_Start:
			TL_state = TL_Blink;
			break;
		case TL_Blink:
			TL_state = TL_Blink_Again;
			break;
		case TL_Blink_Again:
			TL_state = TL_Start;
			break;
		default:
			TL_state = TL_Start;
			break;
	}
	switch(TL_state){
		case TL_Start:
			threeLED = 0x01;
			hold2 = threeLED;
			break;
		case TL_Blink:
			threeLED = 0x02;
			hold2 = threeLED;
			break;
		case TL_Blink_Again:
			threeLED = 0x04;
			hold2 = threeLED;
			break;
		default:
			break;
	}
}
void TickFct_Butt(){
	
	b = PINA & 0x01;
	
	switch(Butt_state){
		case Butt_Start:
		if(!b){
			Butt_state = Butt_Press;
			break;
		}
		else{
			Butt_state = Butt_Start;
			break;
		}
		case Butt_Press:
			Butt_state = Butt_Hold;
			break;
		case Butt_Hold:
			if(b){
				Butt_state = Butt_Hold;
				break;
			}
			else{
				Butt_state = hold;
			}
		case hold:
			if(!b){
				Butt_state = hold;
				break;
			}
			else{
				Butt_state = Butt_press;
				break;
			}
		case Butt_press:
			Butt_state = Butt_hold;
			break;
		case Butt_hold:
			if(b){
				Butt_state = Butt_hold;
				break;
			}
			else{
				Butt_state = Butt_Start;
				break;
			}
		default:
			Butt_state = Butt_Start;
			break;
	}
	switch(Butt_state){
		case Butt_Start:
			PORTB = (blinkingLED << 4) | threeLED;
			break;
		case Butt_Press:
			break;
		case Butt_Hold:
			break;
		case hold:
			PORTB = (hold1 << 4) | hold2;
			break;
		case Butt_press:
			break;
		case Butt_hold:
			break;
		default:
			break;
	}
}
void Combine_Fct(){
	b = PINA & 0x01;
	switch(Com_states){
		case Com_Start:
			Com_states = Com_Action;
			break;
		case Com_Action:
			Com_states = Com_Action;
			break;
		default:
			Com_states = Com_Start;
			break;
	}
	switch(Com_states){
		case Com_Start:
			break;
		case Com_Action:
			PORTB = (blinkingLED << 4) | threeLED;
			break;
		default:
			break;
	}
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	
	unsigned long BL_elapsedTime = 0;
	unsigned long TL_elapsedTime = 0;
	unsigned long Butt_elapsedTime = 0;
	const unsigned long timerPeriod = 100;
	
	TimerSet(timerPeriod);
	TimerOn();
	
	PORTB = blinkingLED | threeLED;
	
	BL_state = BL_Start;
	TL_state = TL_Start;
	Butt_state = Butt_Start;
	Com_states = Com_Start;

	while(1){
		if(BL_elapsedTime >= 1000){
			TickFct_BL();
			BL_elapsedTime = 0;
		}
		if(TL_elapsedTime >= 300){
			TickFct_TL();
			TL_elapsedTime = 0;
		}
		if(Butt_elapsedTime >= 2){
			TickFct_Butt();
			Butt_elapsedTime = 0;
		}
		/*Combine_Fct();*/
		/*PORTB = (blinkingLED << 4) | threeLED;*/
		while(!TimerFlag){}
		TimerFlag = 0;
		BL_elapsedTime += timerPeriod;
		TL_elapsedTime += timerPeriod;
		Butt_elapsedTime += 1;
	}
}