/*
 * clega001_jcent001_lab4_part2.c
 *
 * Created: 10/17/2019 6:58:02 PM
 * Author : Christian Legaspino
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
enum LED1State {init1, on1, off1} led1_state;
enum LED2State {init2, on2, off2} led2_state;
enum LED3State {init3, on3, off3} led3_state;

void LED1_Init(){
	led1_state = init1;
}
void LED2_Init(){
	led2_state = init2;
}
void LED3_Init(){
	led3_state = init3;
}
void LED1_Tick(){
	//Actions
	unsigned char led1 = 0x00;
	switch(led1_state){
		case init1:
		PORTD = 0;
		break;
		case on1:
		led1 = 0x01;
		PORTD |= led1;
		break;
		case off1:
		PORTD &= 0xFE;
		default:
		PORTD = 0;
		break;
	}
	//Transitions
	switch(led1_state){
		case init1:
		led1_state = on1;
		break;
		case on1:
		led1_state = off1;
		break;
		case off1:
		led1_state = on1;
		break;
		default:
		led1_state = init1;
		break;
	}
}
void LED2_Tick(){
	//Actions
	unsigned char led2 = 0x00;
	switch(led2_state){
		case init2:
		PORTD = 0;
		break;
		case on2:
		led2 = 0x04;
		PORTD |= led2;
		break;
		case off2:
		PORTD &= 0xFB;
		default:
		PORTD = 0;
		break;
	}
	//Transitions
	switch(led2_state){
		case init2:
		led2_state = on2;
		break;
		case on2:
		led2_state = off2;
		break;
		case off2:
		led2_state = on2;
		break;
		default:
		led2_state = init2;
		break;
	}
}
void LED3_Tick(){
	//Actions
	unsigned char led3 = 0x00;
	switch(led3_state){
		case init3:
		PORTD = 0;
		break;
		case on3:
		led3 = 0x10;
		PORTD |= led3;
		break;
		case off3:
		PORTD &= 0xEF;
		default:
		PORTD = 0;
		break;
	}
	//Transitions
	switch(led1_state){
		case init3:
		led3_state = on3;
		break;
		case on3:
		led3_state = off3;
		break;
		case off3:
		led3_state = on3;
		break;
		default:
		led3_state = init3;
		break;
	}
}
void Led1SecTask() {
	LED1_Init();
	for(;;) {
		LED1_Tick();
		vTaskDelay(500);
	}
}
void Led2SecTask() {
	LED2_Init();
	for(;;) {
		LED2_Tick();
		vTaskDelay(1000);
	}
}
void Led3SecTask() {
	LED3_Init();
	for(;;) {
		LED3_Tick();
		vTaskDelay(2500);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority) {
	xTaskCreate(Led1SecTask,
	(signed portCHAR *)"Led1SecTask",
	configMINIMAL_STACK_SIZE,
	NULL,
	Priority,
	NULL );
	xTaskCreate(Led2SecTask,
	(signed portCHAR *)"Led2SecTask",
	configMINIMAL_STACK_SIZE,
	NULL,
	Priority,
	NULL );
	xTaskCreate(Led3SecTask,
	(signed portCHAR *)"Led3SecTask",
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

