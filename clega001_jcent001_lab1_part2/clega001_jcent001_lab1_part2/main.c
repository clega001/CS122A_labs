/*
 * Christian Legaspino (clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 1 Exercise 2
 * Lab Description: Press button at certain time with alternating LEDs. Display score on LCD.
 * 
 * I acknowledge all content contained herein, excluding template 
 *		or example code, is my own original work.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"

enum States{Start, Init, Blink1Wait, Blink2, Blink2Wait, Blink3, Blink3Wait, CheckScore} state;

unsigned char button;
unsigned char value = 5;

void Tick(){
	button = (~PINA) & 0x01;
	switch(state){
		case Start:
		LCD_Cursor(1);
		LCD_WriteData( value + '0' );
		state = Init;
		break;
		case Init:
		LCD_Cursor(1);
		LCD_WriteData( value + '0' );
		if ((button) == 0x01){
			state = Blink1Wait;
		}
		else{ state = Blink2; }
		break;
		case Blink1Wait:
		if((button) == 0x00){
			value--;
			LCD_Cursor(1);
			LCD_WriteData( value + '0' );
			state = Init;
		}
		else{
			state = Blink1Wait;
		}
		break;
		case Blink2:
		LCD_Cursor(1);
		LCD_WriteData( value + '0' );
		if ((button) == 0x01){
			state = Blink2Wait;
		}
		else {state = Blink3;}
		break;
		case Blink2Wait:
		if((button) == 0x00){
			value++;
			LCD_Cursor(1);
			LCD_WriteData( value + '0' );
			state = CheckScore;
		}
		else{
			state = Blink2Wait;
		}
		break;
		case Blink3:
		LCD_Cursor(1);
		LCD_WriteData( value + '0' );
		if ((button) == 0x01){
			state = Blink3Wait;
		}
		else {state = Init;}
		break;
		case Blink3Wait:
		if((button) == 0x00){
			value--;
			LCD_Cursor(1);
			LCD_WriteData( value + '0' );
			state = Blink3;
		}
		else{
			state = Blink3Wait;
		}
		break;
		case CheckScore:
		if(value == 9){
			LCD_DisplayString(1, "WINNER");
			state = CheckScore;
		}
		else{state = Blink2;}
		break;
		default:
		state = Start;
		break;
	}
	switch(state){
		case Start:
		break;
		case Init:
		PORTB = 0x01;
		break;
		case Blink1Wait:
		PORTB = 0x01;
		break;
		case Blink2:
		PORTB = 0x02;
		break;
		case Blink2Wait:
		PORTB = 0x02;
		break;
		case Blink3:
		PORTB = 0x04;
		break;
		case Blink3Wait:
		PORTB = 0x04;
		break;
		case CheckScore:
		break;
		default:
		break;
	}
}

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

int main(void)
{
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;

	LCD_init();
	TimerSet(500);
	TimerOn();
	
	state = Start;

	while (1)
	{
		Tick();
		while(!TimerFlag){}
		TimerFlag = 0;
		continue;
	}
}
