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
/* interpolate. */
#define IP(y, y0, x0, y1, x1) ((y - y0) * (x1 - x0)) / (y1 - y0) + x0

uint8_t
get_humidity (short pin)
{
  float voltage;
  uint32_t resistance;

  ADMUX |= pin;
  ADCSRA |= _BV (ADSC);
  _delay_ms (1);
  voltage = (5.05 / 1024) * ADC;
  resistance = (voltage * 1040000) / (5.05 - voltage);

  if (resistance <= 5764705 && resistance >= 2588235)
    return IP (resistance, 2588235, 27.27, 5764705, 22.72);
  else if (resistance <= 2588235 && resistance >= 1000000)
    return IP (resistance, 1000000, 30, 2588235, 27.27);
  else if (resistance <= 1000000 && resistance >= 629411)
    return IP (resistance, 629411, 37.27, 1000000, 30);
  else if (resistance <= 629411 && resistance >= 100000)
    return IP (resistance, 100000, 48.63, 629411, 37.27);
  else if (resistance <= 100000 && resistance >= 84117)
    return IP (resistance, 84117, 52.27, 100000, 48.63);
  else if (resistance <= 84117 && resistance >= 47058)
    return IP (resistance, 47058, 62.72, 84117, 52.27);
  else if (resistance <= 47058 && resistance >= 10000)
    return IP (resistance, 10000, 72.72, 47058, 62.72);
  else if (resistance <= 10000 && resistance >= 3117)
    return IP (resistance, 3117, 92.72, 10000, 72.72);
  /* we're dead anyway, so who cares. */
  return 0;
}
