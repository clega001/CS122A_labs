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

unsigned short t = 0x0000;
unsigned short k = 0x0000;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//taken from http://extremeelectronics.co.in/avr-tutorials/using-adc-of-avr-microcontroller/
void ADC_init()
{
	ADMUX=(1<<REFS1)|(1<<REFS0);                         // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
}
uint16_t ReadADC(uint8_t ch)
{
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	
	ADMUX|=ch;
	
	// I added here: allows time to switch between analog channels
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
// Implement scheduler code from PES.
int main(){
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

 	//Init ADC for Interrupt
// 	ADCSRA |= 1<<ADPS2; //Set ADC Pre-scaler
// 	ADMUX |= 1<<REFS0 | 1<<REFS1; // Voltage Reference 
// 	ADCSRA |= 1<<ADIE; //ADC Interrupt Flag
// 	ADCSRA |= 1<<ADEN; //ADC Enable
// 
// 	sei();
// 	
// 	ADCSRA |= (1<<ADSC); //Start ADC Conversion 
	
	
	ADC_init();
	ADMUX = 0xC1;
	ADCSRA |= (1<<ADSC); //Start ADC Conversion
	_delay_ms(5); 
	

	while(1) {
		t = ReadADC(1);
		//t = ADC;
			if(t > 240){
				PORTB = 0x01;
			}
			else if(t > 100){
				PORTB = 0x02;
			}
			else{
				PORTB = 0x04;
			}	
			
		_delay_ms(10);
		ADMUX = 0xC2;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		
		k = ReadADC(2);
		//k = ADC;
			if(k > 240){
				PORTD = 0x01;
			}
			else if(k > 100){
				PORTD = 0x02;
			}
			else{
				PORTD = 0x04;
			}

		_delay_ms(10);
		ADMUX = 0xC1;
		ADCSRA |= (1<<ADSC);
		_delay_ms(5);
		
		
		
		
		
		
		
		
// 		t = ReadADC(1);
// 		if(t > 240){
// 			PORTB = 0x01;
// 		}
// 		else if(t > 100){
// 			PORTB = 0x02;
// 		}
// 		else{
// 			PORTB = 0x04;
// 		}
//		_delay_ms(10);
// 		k = ReadADC(2);
// 		if(k > 240){
// 			PORTD = 0x01;
// 		}
// 		else if(k > 100){
// 			PORTD = 0x02;
// 		}
// 		else{
// 			PORTD = 0x04;
// 		}
		
		
		
		
	}
	return 0;
}