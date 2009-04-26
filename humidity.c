/* Copyright 2009 Evgeny Grablyk <evgeny.grablyk@gmail.com>
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <avr/io.h>
#include <humidity.h>
#include <util/delay.h>

void adc_setup(void){
	ADMUX |= _BV(REFS0) | _BV(ADLAR);
	ADCSRA |= _BV(ADPS2) | _BV(ADPS0) | _BV(ADEN); 
}

void adc_startc(short pin){
	ADMUX = 0; //reset pin setting bits
	ADMUX |= pin;
	adc_setup();
	_delay_us(130);
	ADCSRA |= _BV(ADSC);
	_delay_ms(15);//start conversion, pin is set
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
