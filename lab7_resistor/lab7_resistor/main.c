/*
 * lab7_resistor.c
 *
 * Created: 10/30/2019 10:13:33 PM
 * Author : Christian Legaspino
 */ 

#include <avr/io.h>
#include <timer.h>

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
void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}
//--------Task scheduler data structure---------------------------------------
typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

//--------End Task scheduler data structure-----------------------------------
//--------Shared/Global Variables----------------------------------------------------
unsigned short tmp = 0x0000;
unsigned short send = 0x0000;
//--------End Shared/Global Variables------------------------------------------------
//Servant
void SPI_SlaveInit(void){
	/* Set MISO output, all others input */
	
	/*DDR_SPI = (1<<DD_MISO);*/
	PORTB = (1<<PORTB4)|(1<<PORTB6);
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

char SPI_SlaveSend(short cData){
	/*Start transmission*/
	SPDR = cData;
	/*Wait for transmission complete*/
	//while(!(SPSR & (1<<SPIF)))
	;

}
//--------User defined FSMs---------------------------------------------------
enum SM1_States{start, wait};
int SM1Tick(int state){
	
	switch(state){
		case start:
			state = wait;
			break;
		case wait:
			state = wait;
			break;
		default:
			state = start; break;
	}
	switch(state){
		case start:
			break;
		case wait:
			tmp = ADC;
			PORTC = tmp;
			SPI_SlaveSend(tmp);
			break;
		default:
			break;
	}
	return state;
}


// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;

	// Period for the tasks
	unsigned long int SMTick1_calc = 1000;

	//Calculating GCD
	unsigned long int tmpGCD = 1;
	// 	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	// 	tmpGCD = findGCD(tmpGCD, SMTIck3_calc);

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
	task1.TickFct = &SM1Tick;//Function pointer for the tick.


	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	
	ADC_init();
	SPI_SlaveInit();
		
	unsigned short i;
	while(1) {
		for ( i = 0; i < numTasks; i++ ) {
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}

