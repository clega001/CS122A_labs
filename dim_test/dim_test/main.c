/*
 * dim_test.c
 *
 * Created: 11/14/2019 7:24:41 PM
 * Author : Christian Legaspino
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define on PORTB |= (1<<PORTB5)
#define off PORTB &= ~(1<<PORTB5)
#define toggle PINB |= (1<<PINB5)

ISR(TIMER1_COMPA_vect){
	on;
}

ISR(TIMER1_COMPB_vect){
	off;
}

void timerFreq(uint_8 freq){
	TCCR1B |= (1<<CS12)|(1<<WGM12);
	TIMSK1 |= (1<<OCIE1A);
	
	OCR1A = (F_CPU/(freq*2*256)-1);
}

void PWM_init(void){
	TCCR1B |= (1<<CS10)|(1<<WGM12);
	TIMSK1 |= (1<<OCIE1A)|(1<<OCIE1B);
	
	OCR1A = 800;
	OCR1B = 400;
}

int main(void){
	DDRB
}