/*
 *      humidity.c
 * 
 * 		Humidity measurement module.
 *      
 *      Copyright 2009  <sm@linuxforum.org.ua>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
*/

#include <avr/io.h>
#include <humidity.h>

void adc_setup(void){
	ADMUX |= _BV(REFS0) | _BV(ADLAR);
	ADCSRA |= _BV(ADPS2) | _BV(ADPS0) | _BV(ADEN); 
}

void adc_startc(short pin){
	ADMUX = 0; //reset pin setting bits
	//set measurement pin
	switch(pin){
		case 0:
		break;
		case 1:
		ADMUX = 1;
		case 2:
		ADMUX = 1 << 1;
		case 3:
		ADMUX = 1 | 1 << 1;
		case 4:
		ADMUX = 1 << 2;
		case 5:
		ADMUX = 1 | 1 << 2;
		case 6:
		ADMUX = 1 << 1 | 1 << 2;	
	}
	adc_setup();
	ADCSRA |= _BV(ADSC); //start conversion, pin is set
	while(ADCSRA & _BV(ADSC))
	  ;;
}

int mhumid(short pin){
  float u, relH, res;
  
	adc_startc(pin);
	u = (5.07 / 256) * ADCH;
	res = (4600 * u) / (5.07 - u);
	
	if(res >= 1000000)
		relH = 30 - res / 529100;
	else if(res >= 100000 && res <= 1000000)
		relH = 48 - res / 50000;
	else if(res <= 100000 && res >= 10000)
		relH = 75 - res / 3333;
	else if(res <= 10000 && res >= 200)
		relH = 95 - res / 490;
	
	return relH * 10;
}
