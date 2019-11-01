/*
 * Christian Legaspino clega001@ucr.edu) & Joshua Centeno (jcent001@ucr.edu)
 * Lab Section: 23
 * Assignment: Lab 4 Exercise 3
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
enum LEDState {INIT,L0,L1} led_state;

void LEDS_Init(){
	led_state = INIT;
}

void LEDS_Tick(){
	//Actions
	switch(led_state){
		case INIT:
		PORTD = 0x01;
		break;
		case L0:
		PORTD = PORTD << 1;
		break;
		case L1:
		PORTD = PORTD >> 1;
		break;
		default:
		PORTD = 0;
		break;
	}
	//Transitions
	switch(led_state){
		case INIT:
		led_state = L0;
		break;
		case L0:
		if(PORTD != 0x80){
			led_state = L0;
			break;
		}
		else{
			led_state = L1;
			break;
		}
		case L1:
		if(PORTD != 0x01){
			led_state = L1;
			break;
		}
		else{
			led_state = L0;
			break;
		}
		default:
		led_state = INIT;
		break;
	}
}

void LedSecTask() {
	LEDS_Init();
	for(;;) {
		LEDS_Tick();
		vTaskDelay(100);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority) {
	xTaskCreate(LedSecTask,
	(signed portCHAR *)"LedSecTask",
	configMINIMAL_STACK_SIZE,
	NULL,
	Priority,
	NULL );
}

int main(void) {
	DDRA = 0x00; PORTA=0xFF;
	DDRD = 0xFF;
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	return 0;
}

