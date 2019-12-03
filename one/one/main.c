/*
 * one.c
 *
 * Created: 11/12/2019 5:53:51 PM
 * Author : Christian Legaspino
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <timer.h>
#include <util/delay.h>
#include "pwm.c"

unsigned short a, b, c, d, e, f, g, h;
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



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;

	ADC_init();
	
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
			}
			else{
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
			}
			else{
				PORTC = 0x0E;
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
				PORTC = 0x00;
			}
			else{
				PORTC = 0x01;
			}
	}
	return 0;
}