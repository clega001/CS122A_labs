/*
 * two_test.c
 *
 * Created: 11/19/2019 7:53:16 PM
 * Author : Christian Legaspino
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"


int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0x00; PORTD = 0xFF;
	
	initUSART(0);
	USART_Flush(0);
	unsigned char t = 0x00;

    while (1) 
    {
		if(USART_HasReceived(0))
		{
			t = USART_Receive(0);
			USART_Flush(0);
			_delay_ms(50);
		}
		PORTB = t;
    }
}

