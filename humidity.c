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

float u, relH, res;

void
adc_setup (void)
{
  ADMUX |= _BV (REFS0);
  ADCSRA |= _BV (ADPS2) | _BV (ADPS1) | _BV (ADPS0) | _BV (ADEN);
}

uint16_t
get_humidity (short pin)
{
  ADMUX |= pin;
  ADCSRA |= _BV (ADSC);
  _delay_ms (1);
  u = (5.07 / 1024) * ADC;
  res = (6000 * u) / (5.07 - u);

  if (res >= 1000000)
    relH = 30 - res / 529100;
  else if (res >= 100000 && res <= 1000000)
    relH = 48 - res / 50000;
  else if (res <= 100000 && res >= 10000)
    relH = 75 - res / 3333;
  else if (res <= 10000 && res >= 200)
    relH = 95 - res / 490;

  return relH * 10;
}
