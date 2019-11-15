/*
 * lab7_resistor.c
 *
 * Created: 10/30/2019 10:13:33 PM
 * Author : Christian Legaspino
 */ 

#include <avr/io.h>
#include <timer.h>
#include <avr/interrupt.h>
#include <string.h>
#include <math.h>

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
unsigned short r = 0x0000;
//--------End Shared/Global Variables------------------------------------------------
int countMSB(short msb){
	unsigned char n = msb;
	unsigned char bitpos = 0;
	
	while(n != 0){
		bitpos++;
		n = n >> 1;
	}
	switch(bitpos){
		case 0: return 0;
		case 1: return 1;
		case 2: return 2;
		case 3: return 3;
		case 4: return 4;
		case 5: return 5;
		case 6: return 6;
		case 7: return 7;
		case 8: return 8;
		case 9: return 9;
	}
	//return bitpos;
}
//Master
void SPI_MasterInit(void){
	/* Set MOSI and SCK output, all others input */
	
	/*DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK); */
	DDRB = (1<<DDRB4)|(1<<DDRB5)|(1<<DDRB7);

	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}
void SPI_MasterTransmit(short cData){
	/* Start transmission */
	//SPDR = cData;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}
//Servant
void SPI_SlaveInit(void){
	/* Set MISO output, all others input */

	/*DDR_SPI = (1<<DD_MISO);*/
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
int hex2Dec(unsigned short t){
	int dec = 0, remain = 0, count = 0;
	while(t != 0){
		remain = t % 10;
		dec = dec + remain * pow(16,count);
		t = t/10;
		count++;
	}
	return dec;
}
//--------User defined FSMs---------------------------------------------------
// tmp = ADC;
// send = countMSB(tmp);
// SPDR = 0x04;
// SPDR = tmp;
// PORTD = send;
//----------------------------------------------------------------------------
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
			PORTD = tmp;
			/*tmp = hex2Dec(tmp);*/
			//tmp = countMSB(tmp);
			SPDR = tmp;
			break;
		default:
			break;
	}
	return state;
}

enum SM2_States{start2, wait2} state2;
int SM2Tick(int state2){
	switch(state2){
		case start2:
			state2 = wait2;
			break;
		case wait2:
			state2 = wait2;
			break;
		default:
			state2 = start2; break;
	}
	switch(state2){
		case start2:
			break;
		case wait2:
			//r = SPI_SlaveReceive();
			break;
		default:
			break;
	}
	return state2;
}
void enableInterrupts(void){
	SPCR |= (1<<SPIE);
	SPCR |= (1<<SPE);
	PCICR |= (1 << PCIE1);
	PCMSK1 |= (1 << PCINT13);
	PCMSK1 |= (1 << PCINT14);
}

ISR(PCINT1_vect){
	/*SPDR = tmp;*////r = SPI_SlaveReceive();
	//PORTD = 0xFF;
}
// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	// Period for the tasks
	unsigned long int SMTick1_calc = 1000;
	unsigned long int SMTick2_calc = 50;

	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	// 	tmpGCD = findGCD(tmpGCD, SMTIck3_calc);

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
	task1.TickFct = &SM1Tick;//Function pointer for the tick.

	// Task 2
	task2.state = -1;
	task2.period = SMTick2_period;
	task2.elapsedTime = SMTick2_period;
	task2.TickFct = &SM2Tick;

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	
	ADC_init();
	//SPI_MasterInit();
	SPI_SlaveInit();
	enableInterrupts();
	sei();
		
// 	unsigned short i;
// 	while(1) {
// 		for ( i = 0; i < numTasks; i++ ) {
// 			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
// 				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
// 				tasks[i]->elapsedTime = 0;
// 			}
// 			tasks[i]->elapsedTime += 1;
// 		}
// 		while(!TimerFlag);
// 		TimerFlag = 0;
// 	}

	TimerSet(100);
	TimerOn();
	while(1){
		//r = SPI_SlaveReceive();
	
// 		if(SPDR == 0x10){
// 			tmp = ADC;
// 			SPDR = tmp;
// 			PORTD = tmp;
// 		} else{
// 			SPDR = 0x02;
// 			PORTD = 0xAA;
// 		}
		tmp = ADC;
		SPDR = tmp;
	}



	return 0;
}

