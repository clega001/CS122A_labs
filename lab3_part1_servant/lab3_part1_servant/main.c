/*
 * Christian Legaspino (clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 3 Exercise 1
 * Lab Description: 
 * 
 * I acknowledge all content contained herein, excluding template 
 *		or example code, is my own original work.
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <bit.h>
#include <timer.h>
#include <stdio.h>

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

//--------End Shared Variables------------------------------------------------
//Master
void SPI_MasterInit(void){
	/* Set MOSI and SCK output, all others input */
	
	/*DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK); */
	PORTB = (1<<PORTB4)|(1<<PORTB5)|(1<<PORTB7)|( DDRB &~(1<<PORTB6));

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
//--------User defined FSMs---------------------------------------------------
enum States{Start, Wait, Key_In} state;
	
int TickFct_Keypad(int state){
	switch(state){
		case Start:
			state = Wait; break;
		case Wait: 
			state = Wait; break;
		default:
			state = Wait; break;
	}
	switch(state){
		case Start:
			break;
		case Wait:
		    PORTA = 0x02;
			PORTD = SPI_SlaveReceive();
		default:
			break;
	}
	return state;
}
// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
// Set Data Direction Registers

DDRA = 0xFF; PORTA = 0x00;
DDRD = 0xFF; PORTD = 0x00;
DDRB = 0x00; PORTB = 0x00;

// Period for the tasks
unsigned long int SMTick1_calc = 500;

//Calculating GCD
unsigned long int tmpGCD = 1;
tmpGCD = findGCD(tmpGCD, SMTick1_calc);

//Greatest common divisor for all tasks or smallest time unit for tasks.
unsigned long int GCD = tmpGCD;

//Recalculate GCD periods for scheduler
unsigned long int SMTick1_period = SMTick1_calc/GCD;


//Declare an array of tasks 
static task task1;
task *tasks[] = {&task1};
const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

// Task 1
task1.state = -1;//Task initial state.
task1.period = SMTick1_period;//Task Period.
task1.elapsedTime = SMTick1_period;//Task current elapsed time.
task1.TickFct = &TickFct_Keypad;//Function pointer for the tick.

// Set the timer and turn it on
TimerSet(GCD);
TimerOn();

SPI_SlaveInit();
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
	while(!TimerFlag);
	TimerFlag = 0;
}

// Error: Program should not exit!
return 0;
}

