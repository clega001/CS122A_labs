/*
 * three_alt.c
 *
 * Created: 12/2/2019 12:52:56 AM
 * Author : Christian Legaspino
 */ 

//Includes
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
unsigned short a, b, c, d, e, f, g, h;
unsigned short cnt = 0;

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

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

//taken from http://extremeelectronics.co.in/avr-tutorials/using-adc-of-avr-microcontroller/
void ADC_init()
{
	ADMUX=(1<<REFS1)|(1<<REFS0);                         // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
}
uint16_t ReadADC(uint8_t ch){//I added the small delays
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	ADMUX|=ch;
	_delay_ms(1);
	
	//Start Single conversion
	ADCSRA|=(1<<ADSC);
	_delay_ms(1);
	
	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));
	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!
	ADCSRA|=(1<<ADIF);
	return(ADC);
}

void PWM_init(){
	TCCR0A = (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00);
	TCCR0B = (1<<CS00);
	TCCR1A =  (1<<COM1A1)|(1<<COM1A0)|(1<<COM1B1)|(1<<COM1B1)|(1<<WGM11)|(1<<WGM10);
	TCCR1B = (1<<CS00);
	TCCR2A = (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00);
	TCCR2B = (1<<CS00);
	TCCR3A = (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00);
	TCCR3B = (1<<CS00);
	OCR0A = 128;
	OCR0B = 128;
	OCR1A = 128;
	OCR1B = 128;
	OCR2A = 128;
	OCR2B = 128;
	OCR3A = 128;
	OCR3B = 128;
}

//State Machines
enum blinkState{init} state;
int blink(int state){
	switch(state){
		case init:
			ADMUX = 0xC0;
			ADCSRA |= (1<<ADSC);
			_delay_ms(5);
		
			a = ReadADC(0);
			if(a > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0xE0;
			}
		
			ADMUX = 0xC1;
			ADCSRA |= (1<<ADSC);
			_delay_ms(5);
		
			b = ReadADC(1);
			if(b > 150){
				PORTC = 0x00;
				USART_Send(0x00, 0);
			}
			else{
				PORTC = 0x70;
				USART_Send(0x01, 0);
			}

			ADMUX = 0xC2;
			ADCSRA |= (1<<ADSC);
			_delay_ms(5);
		
			c = ReadADC(2);
			if(c > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0x38;
			}

			ADMUX = 0xC3;
			ADCSRA |= (1<<ADSC);
			_delay_ms(5);
		
			d = ReadADC(3);
			if(d > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0x1C;
			}

			ADMUX = 0xC4;
			ADCSRA |= (1<<ADSC);
			_delay_ms(5);
		
			e = ReadADC(4);
			if(e > 150){
				PORTC = 0x00;
				USART_Send(0x00, 0);
			}
			else{
				PORTC = 0x0E;
				USART_Send(0x02, 0);
			}
		
			ADMUX = 0xC5;
			ADCSRA |= (1<<ADSC);
			_delay_ms(5);
		
			f = ReadADC(5);
			if(f > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0x07;
			}
		
			ADMUX = 0xC6;
			ADCSRA |= (1<<ADSC);
			_delay_ms(5);
		
			g = ReadADC(6);
			if(g > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0x03;
			}
		
			ADMUX = 0xC7;
			ADCSRA |= (1<<ADSC);
			_delay_ms(5);
		
			h = ReadADC(7);
			if(h > 150){
				cnt = 0;
				PORTC = 0x00;
				USART_Send(0x00,0);
			}
			else{
				if(cnt >= 500){
					USART_Send(0x04,0);
					PORTC = 0xFF;
					} else {
					cnt += 1;
					PORTC = 0x01;
				}
			
				//PORTC = 0x01;
			}
			break;
	}
	return state;
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xF0, PORTD = 0x00;

	PWM_init();
	ADC_init();
	initUSART(0);
	USART_Flush(0);

	tasks[0].state = init;
	tasks[0].period = 1;
	tasks[0].elapsedTime = 0;
	tasks[0].TickFct = &blink;
	
	TimerSet(PERIOD);
	TimerOn();
	while (1)
	{
		continue;
	}
}

