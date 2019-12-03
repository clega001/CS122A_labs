/*
 * three.c
 *
 * Created: 12/1/2019 6:52:17 PM
 * Author : Christian Legaspino
 */ 


/*
 * one.c
 *
 * Created: 11/12/2019 5:53:51 PM
 * Author : Christian Legaspino
 */ 

#include "bit.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer.h>
#include <util/delay.h>
#include "pwm.c"
#include "usart_ATmega1284.h"

unsigned short a, b, c, d, e, f, g, h;
unsigned short cnt = 0;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//taken from http://extremeelectronics.co.in/avr-tutorials/using-adc-of-avr-microcontroller/
void ADC_init()
{
	ADMUX=(1<<REFS1)|(1<<REFS0);                         // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
}
uint16_t ReadADC(uint8_t ch){//I added the small delays
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	ADMUX|=ch;
	_delay_ms(1);
	
	//Start Single conversion
	ADCSRA|=(1<<ADSC);
	_delay_ms(1);
	
	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));
	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!
	ADCSRA|=(1<<ADIF);
	return(ADC);
}

void PWM_init(){
	TCCR0A = (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00);
	TCCR0B = (1<<CS00);
	TCCR1A =  (1<<COM1A1)|(1<<COM1A0)|(1<<COM1B1)|(1<<COM1B1)|(1<<WGM11)|(1<<WGM10);
	TCCR1B = (1<<CS00);
	TCCR2A = (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00);
	TCCR2B = (1<<CS00);
	TCCR3A = (1<<COM0A1)|(1<<COM0A0)|(1<<COM0B1)|(1<<COM0B0)|(1<<WGM01)|(1<<WGM00);
	TCCR3B = (1<<CS00);
	OCR0A = 128;
	OCR0B = 128;
	OCR1A = 128;
	OCR1B = 128;
	OCR2A = 128;
	OCR2B = 128;
	OCR3A = 128;
	OCR3B = 128;
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xF0, PORTD = 0x00;
	
	unsigned char data = 0x00;
	
	PWM_init();
	ADC_init();
	initUSART(0);
	USART_Flush(0);
	
	while(1) {
		ADMUX = 0xC0;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		
		a = ReadADC(0);
			if(a > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0xE0;
			}	
			
		ADMUX = 0xC1;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		
		b = ReadADC(1);
			if(b > 150){
				PORTC = 0x00;
				data = SetBit(data, 0, 0);
			}
			else{
				data = SetBit(data, 0, 1);
				PORTC = 0x70;
			}

		ADMUX = 0xC2;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		
		c = ReadADC(2);
			if(c > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0x38;
			}

		ADMUX = 0xC3;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		
		d = ReadADC(3);
			if(d > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0x1C;
			}
			
		ADMUX = 0xC4;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		
		e = ReadADC(4);
			if(e > 150){
				PORTC = 0x00;
				data = SetBit(data, 1, 0);
			}
			else{
				PORTC = 0x0E;
				data = SetBit(data, 1, 1);
			}

		ADMUX = 0xC5;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		f = ReadADC(5);
			if(f > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0x07;
			}
			
		ADMUX = 0xC6;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		
		g = ReadADC(6);
			if(g > 150){
				PORTC = 0x00;
			}
			else{
				PORTC = 0x03;
			}
			
		ADMUX = 0xC7;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		
		h = ReadADC(7);
			if(h > 150){
				cnt = 0;
				PORTC = 0x00;
				data = SetBit(data, 2, 0);
			}
			else{
				if(cnt >= 100){
					data = SetBit(data, 2, 1);
				} else {
					cnt += 1;
					PORTC = 0x01;
				}
				
			}
			USART_Send(data, 0);
			_delay_ms(100);	
	}
	return 0;
}