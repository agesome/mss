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

static float rh, vol, r;

void adc_setup(void){
	ADMUX |= _BV(REFS0) | _BV(ADLAR);
	ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS1) | _BV(ADEN); 
}

void adc_startc(short pin){
	ADMUX = 0; //reset pin setting bits
	//set measurement pin
	switch(pin){
		case 0:
		break;
		case 1:
		ADMUX |= 1;
		case 2:
		ADMUX |= 1 << 1;
		case 3:
		ADMUX |= 1 | 1 << 1;
		case 4:
		ADMUX |= 1 << 2;
		case 5:
		ADMUX |= 1 | 1 << 2;
		case 6:
		ADMUX |= 1 << 1 | 1 << 2;	
	}	//ADMUX |= _BV(REFS0);
	adc_setup();
	ADCSRA |= _BV(ADSC); //start conversion, pin is set
}

int mhumid(short pin){	
	adc_startc(pin);
	vol = (5.07 / 256) * ADCH;
	r = (4600 * vol) / (5.07 - vol);
	
	if(r >= 1000000)
		rh = 30 - r / 529100;
	else if(r >= 100000 && r <= 1000000)
		rh = 48 - r / 50000;
	if(r <= 100000 && r >= 10000)
		rh = 75 - r / 3333;
	else if(r <= 10000 && r >= 200)
		rh = 95 - r / 490;
	
	return rh * 10;
}
