//Servant

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
unsigned char shifty = 0x80;
unsigned char bouncey = 0xC0;
unsigned char pattern = 0x00;
unsigned char speed = 0x00;
unsigned char n = 10000;
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

	switch(p1_state){//Test
		case p1_start:
			p1_state = p1_wait; break;
		case p1_wait:
			if(pattern == 0xA0){
				p1_state = left; break;
				}else{p1_state = p1_wait; break;}
		case left:
			if(pattern == 0xA0 && cnt < n){
				p1_state = left;
				cnt = cnt + 10; 
				break;
			}
			else if(pattern == 0xA0 && cnt >= n){
				p1_state = right;
				cnt = 0;
				break;
			}
			else{p1_state = p1_wait; break;}
		case right:
			if(pattern == 0xA0 && cnt < n){
				p1_state = right;
				cnt = cnt + 10; 
				break;
			}
			else if(pattern == 0xA0 && cnt >= n){
				p1_state = left;
				cnt = 0;
				break;
			}
			else{p1_state = p1_wait; break;}
		default:
			p1_state = p1_start; break;
	}
// 	switch(p1_state){//Transition
// 		case p1_start:
// 			p1_state = p1_wait; break;
// 		case p1_wait:
// 			if(pattern == 0xA0){
// 				p1_state = left; break;
// 				}else{p1_state = p1_wait; break;}
// 		case left:
// 			if(pattern == 0xA0){
// 				p1_state = right; break;
// 			}else{p1_state = p1_wait; break;}
// 		case right:
// 			if(pattern == 0xA0){
// 				p1_state = left; break;
// 			}else{p1_state = p1_wait; break;}
// 		default:
// 			p1_state = p1_start; break;
// 	}
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
			if(pattern == 0xB0 && cnt < n){
				p2_state = on; 
				cnt = cnt + 10;
				break;
			}
			else if(pattern == 0xB0 && cnt >= n){
				p2_state = off;
				cnt = 0;
				break;
			}
			else{p2_state = p2_wait; break;}
		case off:
			if(pattern == 0xB0 && cnt < n){
				p2_state = off; 
				cnt = cnt + 10;
				break;
			}
			else if(pattern == 0xB0 && cnt >= n){
				p2_state = on;
				cnt = 0;
				break;
			}
			else{p2_state = p2_wait; break;}
		default:
			p2_state = p2_start; break;
	}

// 	switch(p2_state){
// 		case p2_start:
// 			p2_state = p2_wait; break;
// 		case p2_wait:
// 			if(pattern == 0xB0){
// 				p2_state = on; break;
// 			}else{p2_state = p2_wait; break;}
// 		case on:
// 			if(pattern == 0xB0){
// 				p2_state = off; break;
// 			}else{p2_state = p2_wait; break;}
// 		case off:
// 			if(pattern == 0xB0){
// 				p2_state = on; break;
// 			}else{p2_state = p2_wait; break;}
// 		default:
// 			p2_state = p2_start; break;
// 	}
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
			if(cnt >= n){
				PORTC = shifty;
				cnt = 0;
				if(shifty == 0x01){
					shifty = 0x80;
				}
				else{
					shifty = shifty >> 1;
				}
				break;
			}else{cnt = cnt + 10; break;}
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
				p4_state = bounce_right; 
				break;
			}
			else if(pattern == 0xD0 && bouncey == 0x03){
				p4_state = bounce_left; 
				break;
			}
			else{
				p4_state = p4_wait; break;
			}
		case bounce_left:
			if(pattern == 0xD0 && bouncey != 0xC0){
				p4_state = bounce_left; 
				break;
			}
			else if(pattern == 0xD0 && bouncey == 0xC0){
				p4_state = bounce_right; 
				break;
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
			if(cnt >= n){
				PORTC = bouncey;
				cnt = 0;
				bouncey = bouncey >> 1;
				break;
			}else{cnt = cnt + 10; break;}
		case bounce_left:
			if(cnt >= n){
				PORTC = bouncey;
				cnt = 0;
				bouncey = bouncey << 1;
				break;
			}else{cnt = cnt + 10; break;}
		default:
			break;
	}
	return p4_state;
}
// --------END User defined FSMs-----------------------------------------------

// Implement scheduler code from PES.
int main()
{
// Set Data Direction Registers
DDRB = 0x00; PORTB = 0x00;
DDRC = 0xFF; PORTC = 0x00;

//Period for the tasks
unsigned long int SMTick1_calc = 1;
unsigned long int SMTick2_calc = 1;
unsigned long int SMTick3_calc = 1;
unsigned long int SMTick4_calc = 1;
unsigned long int SMTick5_calc = 1;
unsigned long int SMTick6_calc = 1;

//Calculating GCD
unsigned long int tmpGCD = 1;
tmpGCD = findGCD(tmpGCD, SMTick1_calc);
tmpGCD = findGCD(tmpGCD, SMTick2_calc);
tmpGCD = findGCD(tmpGCD, SMTick3_calc);
tmpGCD = findGCD(tmpGCD, SMTick4_calc);
tmpGCD = findGCD(tmpGCD, SMTick5_calc);
tmpGCD = findGCD(tmpGCD, SMTick6_calc);

//Greatest common divisor for all tasks or smallest time unit for tasks.
unsigned long int GCD = tmpGCD;

//Recalculate GCD periods for scheduler
unsigned long int SMTick1_period = SMTick1_calc/GCD;
unsigned long int SMTick2_period = SMTick2_calc/GCD;
unsigned long int SMTick3_period = SMTick3_calc/GCD;
unsigned long int SMTick4_period = SMTick4_calc/GCD;
unsigned long int SMTick5_period = SMTick5_calc/GCD;
unsigned long int SMTick6_period = SMTick6_calc/GCD;


//Declare an array of tasks 
static task task1, task2, task3, task4, task5, task6;
task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6};
const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

//Task 1
task1.state = -1;//Task initial state.
task1.period = SMTick1_period;//Task Period.
task1.elapsedTime = SMTick1_period;//Task current elapsed time.
task1.TickFct = &TickFct_Receive;//Function pointer for the tick.

//Task 2
task2.state = -1;
task2.period = SMTick2_period;
task2.elapsedTime = SMTick2_period;
task2.TickFct = &TickFct_P1;

//Task 3
task3.state = -1;
task3.period = SMTick3_period;
task3.elapsedTime = SMTick3_period;
task3.TickFct = &TickFct_P2;

//Task 4
task4.state = -1;
task4.period = SMTick4_period;
task4.elapsedTime = SMTick4_period;
task4.TickFct = &TickFct_P3;

//Task 5
task5.state = -1;
task5.period = SMTick5_period;
task5.elapsedTime = SMTick5_period;
task5.TickFct = &TickFct_P4;

//Task 6
task6.state = -1;
task6.period = SMTick6_period;
task6.elapsedTime = SMTick6_period;
task6.TickFct = &TickFct_Speed;

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
		
		
		if(speed == 0x01){
			n = 20000;
		}
		else if(speed == 0x02){
			n = 1000;
		}
		else if(speed == 0x03){
			n = 100;
		}
		else if(speed == 0x04){
			n = 50;
		}
		else if(speed == 0x05){
			n = 20;
		}
		else if(speed == 0x06){
			n = 1;
		}
		else{
		}
	}
	
	while(!TimerFlag);
	TimerFlag = 0;
}

// Error: Program should not exit!
return 0;
}

