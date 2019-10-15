/*
 * Christian Legaspino clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 3 Exercise 2
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <bit.h>
#include <timer.h>
#include <stdio.h>
#include "keypad.h"
#include "io.c"

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

//--------Task scheduler data structure---------------------------------------
// Struct for Tasks represent a running process in our simple real-time operating system.
typedef struct _task {
    /*Tasks should have members that include: state, period,
        a measurement of elapsed time, and a function pointer.*/
    signed char state; //Task's current state
    unsigned long int period; //Task period
    unsigned long int elapsedTime; //Time elapsed since last task tick
    int (*TickFct)(int); //Task tick function
} task;

//--------End Task scheduler data structure-----------------------------------

//--------Shared Variables----------------------------------------------------
unsigned char x = 0x00;
unsigned char key = 0x00;
//--------End Shared Variables------------------------------------------------
//Master
void SPI_MasterInit(void){
	/* Set MOSI and SCK output, all others input */
	
	/*DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK); */
	DDRB = (1<<DDRB4)|(1<<DDRB5)|(1<<DDRB7);

	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void SPI_MasterTransmit(char cData){
	/* Start transmission */
	SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}
//--------User defined FSMs---------------------------------------------------
enum Keypad_States{k_start, wait, Key_In} k_state;
int TickFct_Keypad(int state){
	x = GetKeypadKey();
	
	switch(k_state){
		case k_start:
			k_state = wait;
			break;
		case wait:
			if(x != 0x1F){
				k_state = Key_In;
				break;
			}
			else{
				k_state = wait;
				break;
			}
		case Key_In:
			k_state = wait;
			break;
		default:
			k_state = k_start;
			break;
	}
	switch(k_state){
		case k_start:
			break;
		case wait:
			break;
		case Key_In:
			key = x;
			x = 0;
			break;
		default:
			break;
	}
	return k_state;
}

enum LCD_States{l_start, display} l_state;
int TickFct_LCD(){
	switch(l_state){
		case l_start:
			l_state = display; break;
		case display:
			l_state = display; break;
		default:
			l_state = l_start; break;
	}
	switch(l_state){
		case l_start:
			break;
		case display:
			LCD_DisplayString(1, "Hey");
			break;
		default:
			break;
	}
	return l_state;
}
// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
// Set Data Direction Registers
// Buttons PORTA[0-7], set AVR PORTA to pull down logic
DDRA = 0xF0; PORTA = 0x0F;
DDRB = 0xFF; PORTB = 0x00;
DDRC = 0xFF; PORTC = 0x00;
DDRD = 0xF0; PORTD = 0x0F;


// Period for the tasks
unsigned long int SMTick1_calc = 50;
unsigned long int SMTick2_calc = 1000;

//Calculating GCD
unsigned long int tmpGCD = 1;
tmpGCD = findGCD(tmpGCD, SMTick1_calc);
tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);

//Greatest common divisor for all tasks or smallest time unit for tasks.
unsigned long int GCD = tmpGCD;

//Recalculate GCD periods for scheduler
unsigned long int SMTick1_period = SMTick1_calc/GCD;
unsigned long int SMTick2_period = SMTick2_calc/GCD;

//Declare an array of tasks 
static task task1, task2;
task *tasks[] = {&task1, &task2};
const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

// Task 1
task1.state = -1;//Task initial state.
task1.period = SMTick1_period;//Task Period.
task1.elapsedTime = SMTick1_period;//Task current elapsed time.
task1.TickFct = &TickFct_Keypad;//Function pointer for the tick.

// Task 2
task2.state = -1;
task2.period = SMTick2_period;
task2.elapsedTime = SMTick2_period;
task2.TickFct = &TickFct_LCD;

// Set the timer and turn it on
TimerSet(GCD);
TimerOn();

LCD_init();

unsigned short i; // Scheduler for-loop iterator
while(1) {
    // Scheduler code
    for ( i = 0; i < numTasks; i++ ) {
        // Task is ready to tick
        if ( tasks[i]->elapsedTime == tasks[i]->period ) {
            // Setting next state for task
            tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
            // Reset the elapsed time for next tick.
            tasks[i]->elapsedTime = 0;
        }
        tasks[i]->elapsedTime += 1;
    }
	switch(key){
		// All 5 LEDs on
		case '\0': PORTB = 0x1F; break;
		// hex equivalent
		case '1': PORTB = 0x01; break;
		case '2': PORTB = 0x02; break;
		case '3': PORTB = 0x03; break;
		case '4': PORTB = 0x04; break;
		case '5': PORTB = 0x05; break;
		case '6': PORTB = 0x06; break;
		case '7': PORTB = 0x07; break;
		case '8': PORTB = 0x08; break;
		case '9': PORTB = 0x09; break;
		case 'A': PORTB = 0x0A; break;
		case 'B': PORTB = 0x0B; break;
		case 'C': PORTB = 0x0C; break;
		case 'D': PORTB = 0x0D; break;
		case '*': PORTB = 0x0E; break;
		case '0': PORTB = 0x00; break;
		case '#': PORTB = 0x0F; break;
		// Should never occur. Middle LED off.
		default: PORTB = 0x1B; break;
		
		while(!TimerFlag);
		TimerFlag = 0;
	}
}

// Error: Program should not exit!
return 0;
}