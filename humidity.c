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

void
adc_setup (void)
{
  ADMUX |= _BV (REFS0);
  ADCSRA |= _BV (ADPS2) | _BV (ADPS1) | _BV (ADPS0) | _BV (ADEN);
}

uint16_t
get_humidity (short pin)
{
  float voltage, r_humidity;
  uint32_t resistance;
  
  ADMUX |= pin;
  ADCSRA |= _BV (ADSC);
  _delay_ms (1);
  voltage = (5.05 / 1024) * ADC;
  resistance = (voltage * 1040000) / (5.05 - voltage);
  
  if (resistance >= 1000000)
    r_humidity = 30 - resistance / 529100;
  else if (resistance >= 100000 && resistance <= 1000000)
    r_humidity = 48 - resistance / 50000;
  else if (resistance <= 100000 && resistance >= 10000)
    r_humidity = 75 - resistance / 3333;
  else if (resistance <= 10000 && resistance >= 200)
    r_humidity = 95 - resistance / 490;
  
  return r_humidity * 10;
}
