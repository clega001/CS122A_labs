/*    Name & Email: Emanuel Halfon / ehalf001@ucr.edu
 *    Partner(s) Name & E-mail: Ivan Lorna / ilorn001@ucr.edu 
 *    Lab Section:  021
 *    Assignment: Lab 6 Part 3
 *    Exercise Description: [optional - include for your own benefit]
 *    
 *    I acknowledge all content contained herein, excluding template 
 *     or example code, is my own original work.
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "io.c"
#include <stdio.h>
#include <avr/wdt.h>


#define A0 (~PINA & 0x01)
#define A1 (~PINA & 0x02)
#define A2 (~PINA & 0x04)

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
volatile unsigned char port_B = 0x00;
unsigned char on_off = 0x00;
// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
unsigned char flag = 0x00;
unsigned char counter = 0x03;
unsigned char DisplayOn = 0x03;
const unsigned long PERIOD = 50;

unsigned char isnotStuck= 0x00;
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
	unsigned char active;
	int (*TickFct)(int); // Task tick function
} task;

task* Triggered;



const unsigned char tasksSize = 6;
task tasks[6];


void DisplayTurnOff(void)
{
// 	tasks[0].active = 0; //increment
// 	tasks[1].active = 0; //decrement
// 	tasks[2].active = 0; //reset
// 	tasks[3].active = 0; //display
	tasks[4].active = 0; //decrease counter
	PORTB = 0x00;
}

void DisplayTurnOn(void)
{
// 	tasks[0].active = 1; //increment
// 	tasks[1].active = 1; //decrement
// 	tasks[2].active = 1; //reset
// 	tasks[3].active = 1; //display
	tasks[4].active = 1; //decrease counter
	PORTB = 0xFF;
}

enum IncrStates {Start_incr, toggle_incr_on, incr, incr_wait};

int IncrToggle(int state)
{
	switch(state)
	{
		case Start_incr:
			state = toggle_incr_on;
			break;
		case toggle_incr_on:
			state = A0 ? incr : toggle_incr_on;
			break;
		case incr:
			state = incr_wait;
			break;
		case incr_wait:
			state = !A0 ? toggle_incr_on : incr_wait;
			break;
		default:
			state = Start_incr; 
	}
	switch(state)
	{
		case incr:
			counter = (counter < 9) ? (counter + 1) : counter; 
			break;
		default:
		counter = counter;
	}
	return state;
}

enum DecrStates {Start_decr, toggle_decr_on, decr, decr_wait};

int DecrToggle(int state)
{
	switch(state)
	{
		case Start_decr:
			state = toggle_decr_on;
			break;
		case toggle_decr_on:
			state = A1 ? decr : toggle_decr_on;
			break;
		case decr:
			state = decr_wait;
			break;
		case decr_wait:
			state = !A1 ? toggle_decr_on : decr_wait;
			break;
		default:
		state = Start_decr;
	}
	switch(state)
	{
		case decr:
			counter = (counter > 0) ? (counter - 1) : counter;
			break;
		default:
			counter = counter;
	}
	return state;
}

enum ResetStates {Start_reset, toggle_reset_on, reset, reset_wait};

int ResetToggle(int state)
{
	switch(state)
	{
		case Start_reset:
			state = toggle_reset_on;
			break;
		case toggle_reset_on:
			state = (A1 && A0) ? reset : toggle_reset_on;
			break;
		case reset:
			state = reset_wait;
			break;
		case reset_wait:
			state = !(A1 && A0) ? toggle_reset_on : reset_wait;
			break;
		default:
			state = Start_reset;
	}
	switch(state)
	{
		case reset:
			counter = 0;
			for(;;);
			break;
		default:
			counter = counter;
	}
	return state;
}

enum DisplayState {null};
int Display(int state)
{
	LCD_Cursor(1);
	LCD_WriteData('0' + counter);
}

enum wakeDisplayStates {null_state};

int wakeDisplay(int state)
{
	if(A2)
	{
		DisplayTurnOn();
		DisplayOn = 0x03;
	}
	Triggered->active = 0;
	return state;
}

enum TurnOffDisplays{states};
int TurnOffDisplay(int state)
{
	if(DisplayOn == 0)
	{
		DisplayTurnOff();
	}
	else
	{
		DisplayOn -= 1;
	}
	return state;
}

void TimerISR()
{
	unsigned char i;
	for (i = 0;i < tasksSize;++i)
	{
		if ((tasks[i].elapsedTime >= tasks[i].period) && tasks[i].active)
		{
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += PERIOD;
	}
}

void InterruptEnable(task* t)
{
	PCICR |= (1 << PCIE0); 
	PCMSK0 |= (1 << PCINT2);
	Triggered = t;
}

 ISR(PCINT0_vect)
 {
	Triggered->active = 1;	 
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

void WDT_Init(void)
{
	unsigned char old_SREG = SREG;
	SREG = 0x00;
	wdt_reset(); 
	MCUSR |= (1 << WDRF);
	WDTCSR |= (1 << WDP1) | (1 << WDP2);
	WDTCSR |= (1 << WDIE);
	wdt_enable(WDTO_8S);
	SREG = old_SREG;
}

void WDT_OFF(void)
{
	unsigned char old_SREG = SREG;
	SREG = 0x00;
	wdt_reset();
	MCUSR  = 0x00;
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = 0x00;
	wdt_disable();
	SREG = old_SREG;
}

ISR(WDT_vect)
{
	
}



int main(void)
{
	WDT_OFF();
	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	PORTB = 0xFF;
	tasks[0].active = 1;
	tasks[0].elapsedTime = 0;
	tasks[0].period = 100;
	tasks[0].TickFct = &IncrToggle;
	tasks[0].state = Start_incr;
	tasks[1].active = 1;
	tasks[1].elapsedTime = 0;
	tasks[1].period = 100;
	tasks[1].TickFct = &DecrToggle;
	tasks[1].state = Start_decr;
	tasks[2].active = 1;
	tasks[2].elapsedTime = 0;
	tasks[2].period = 100;
	tasks[2].TickFct = &ResetToggle;
	tasks[2].state = Start_reset;	
	tasks[3].active = 1;
	tasks[3].elapsedTime = 0;
	tasks[3].period = 100;
	tasks[3].TickFct = &Display;
	tasks[3].state = null;
	tasks[4].active = 1;
	tasks[4].elapsedTime = 0;
	tasks[4].period = 1000;
	tasks[4].TickFct = &TurnOffDisplay;
	tasks[4].state = states;
	tasks[5].active = 0;
	tasks[5].elapsedTime = 0;
	tasks[5].period = 100;
	tasks[5].TickFct = &wakeDisplay;
	tasks[5].state = null_state;
	InterruptEnable(&tasks[5]);
	LCD_init();
	LCD_ClearScreen();
	TimerSet(PERIOD);
	TimerOn();
	LCD_Cursor(1);
	LCD_WriteData('0' + counter);

	WDT_Init();
	while(1)
	{
		wdt_reset();
		continue;
	}
	return 0;
}
