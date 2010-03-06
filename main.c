/* Copyright 2010 Evgeny Grablyk <evgeny.grablyk@gmail.com>
   
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

/*sensor to regulate temperature according to*/
#define TEMP_REG_SENSOR 0
#define TEMP_REG_PORT PORTC
#define TEMP_REG_PIN _BV(PC7)

/* buttons are on same port, that simplifies things. */
#define BUTTON_PIN PIND
#define BUTTON0_BIT PD3
#define BUTTON1_BIT PD4
#define BUTTON_CLICK_DELAY 30

#define H_SENSORS 1
#define T_MAXSENSORS 3
#define B_PRESS_DELAY 13

/* A is the port, B is the pin to check */
#define ISCLEAR(A, B) !(A & (~A ^ _BV(B)) )

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <lcd.h>
#include <humidity.h>
#include <usbdrv.h>
#include <temperature.h>
#include <i2cmaster.h>
#include <lib302dl.h>
#include "lib302dl/defines.h"

/* display buffer */
static char display[LCD_DISP_LENGTH * LCD_LINES + 2];
/* pointers to lines, they're set in configure () */
static char *d_status, *d_content;
/* change indicators, so display is not constantly updated */
static uint8_t d_status_ch = 0, d_content_ch = 0;
/* delay between usbPoll () calls */
static uint16_t usb_delay = 0;
/* number of detected temperature sensors, set in configure () */
static uint8_t t_sensors_count = 0;
/* temperature and humidity data */
/* number of sensors is unknown at compile time, so just pick some number. */
static float t_val[T_MAXSENSORS];
static uint8_t h_val[H_SENSORS];
/* acceleration data */
static int16_t accel[3];
/* uptime indicates, uh, uptime (in seconds). uptime_cnt is evil, don't touch it */
static uint16_t uptime_cnt = 0, uptime = 0;
/* enumeration of accel array contents ;) */
/* delay between repeated detection (when buttons are kept pressed) */
static uint8_t b0_press_delay = 0, b1_press_delay = 0;
/* states of buttons */
static uint8_t button_0 = 0, button_1 = 0;
/* "was up" indication. shows if button was pressed at previous check */
static uint8_t b0_was_up = 0, b1_was_up = 0;
/* accelerometer vectors */
enum xyz
{ X, Y, Z };
/* number of taps detected by accelerometer */
static uint16_t accel_taps = 0;
/* sensor presence indication */
static uint8_t have_ts = 0, have_ac = 0;

void d_update (void);
void d_status_update (char *content, ...);
void d_content_update (char *content, ...);

void
configure (void)
{
  /* usb stuff. make the host re-enumerate out device */
  usbDeviceDisconnect ();
  _delay_ms (100);
  usbDeviceConnect ();
  usbInit ();
  sei ();
  /* end of usb stuff  */

  /* configure timer 2 for calling usbPoll() */
  TIMSK2 = _BV (TOIE2);		/* overflow interrupt enabled */
  TCCR2B = _BV (CS02) | _BV (CS01) | _BV (CS00);	/* set prescaler to 1024, timer starts */

  /* display configuration */
  d_status = &display[0];
  display[LCD_DISP_LENGTH] = '\0';
  d_content = &display[LCD_DISP_LENGTH + 1];
  display[LCD_DISP_LENGTH * 2 + 1] = '\0';

  lcd_init (LCD_DISP_ON);
  /* end display configuration */

  /* temperature sensors */
  t_sensors_count = search_sensors ();
  d_status_update ("Temp. sensors:");
  if (t_sensors_count)
    {
      d_content_update ("%d found.", t_sensors_count);
      have_ts = 1;
    }
  else
    d_content_update ("not found.");
  d_update ();
  _delay_ms (1000);
  /* end if temperature sensors setup */

  /* configure timer 0 for button state detection */
  TIMSK0 = _BV (TOIE0);		/* enable overflow interrupt */
  TCCR0B = _BV (CS02) | _BV (CS00);	/* set prescaler to 1024, timer starts */
  PORTD |= _BV (PD3) | _BV (PD4);	/* pullup for button 0 */
  /* end if button detection setup */

  /* clock setup */
  TIMSK1 = _BV (OCIE1A);
  /* reaching this with a prescaler of 256 and frequency of 20Mhz five times is exactly one second */
  OCR1A = 15625;
  TCCR1B = _BV (CS12) | _BV (WGM12);
  /* end of clock setup */
  
  /* ADC configuration goes here */
  ADMUX = _BV (REFS0);
  ADCSRA = _BV (ADEN) | _BV (ADPS2) | _BV (ADPS1) | _BV (ADPS0);
  PORTC |= _BV (PC0) | _BV (PC1);
  /* end of ADC configuration */

  /* twi/accelerometer configuration */
  i2c_init ();
  d_status_update ("Accelerometer:");
  if (lis_initialize (0, 1, 1, 1))
    d_content_update ("not found.");
  else
    {
      d_content_update ("found.");
      have_ac = 1;
    }
  d_update ();
  _delay_ms (1000);
  /* end of accelerometer configuration */

  adc_setup ();

  /* clear the display */
  d_content_update (" ");
  d_status_update (" ");
  d_update ();
}

ISR (TIMER1_COMPA_vect, ISR_NOBLOCK)
{
  uptime_cnt++;

  if (uptime_cnt == 5)
    {
      uptime++;
      uptime_cnt = 0;
    }
}

/* button handling */
/* meant to be working like this:
   <signal>       -------------
   <button state> -___-___-___-
 */
ISR (TIMER0_OVF_vect, ISR_NOBLOCK)
{
  uint8_t bit, *button, *delay, *wasup, run = 0;
  
 run:
  if (!run)
    {
      bit = BUTTON0_BIT;
      button = &button_0;
      delay = &b0_press_delay;
      wasup = &b0_was_up;
      run++;
      goto check;
    }
  else if (run == 1)
    {
      bit = BUTTON1_BIT;
      button = &button_1;
      delay = &b1_press_delay;
      wasup = &b1_was_up;
      run++;
      goto check;
    }
  else
    goto ret;
    
 check:
  if (ISCLEAR (BUTTON_PIN, bit))
    {
      (*delay)++;
      if (*wasup && *delay >= BUTTON_CLICK_DELAY)
	{
	  *button = 1;
	    *delay = 0;
	    *wasup = 0;
	    goto exit;
	}
      else if (!*wasup)
	{
	  *wasup = 1;
	  *delay = 0;
	  goto exit;
	}
      else
	goto exit;
    }
  *wasup = 0;
  *delay = 0;
 exit:
  goto run;
 ret:
  return;
}

ISR (TIMER2_OVF_vect, ISR_BLOCK)
{
  usb_delay++;

  /* timer 2 overflows in ~13 msec, so we'll call usbPoll() each 26 msec, which is just fine */
  if (usb_delay == 2)
    {
      usbPoll ();
      usb_delay = 0;
    }
}

void
d_update (void)
{
  int unused;

  if (d_status_ch)
    {
      unused = 16 - strlen (d_status);
      lcd_home ();
      lcd_puts (d_status);
      while (unused--)
	lcd_putc (' ');
      d_status_ch = 0;
      lcd_home ();
    }
  if (d_content_ch)
    {
      unused = 16 - strlen (d_content);
      lcd_gotoxy (0, 2);
      lcd_puts (d_content);
      while (unused--)
	lcd_putc (' ');
      d_content_ch = 0;
      lcd_home ();
    }
}

void
d_status_update (char *content, ...)
{
  va_list ap;

  va_start (ap, content);
  vsnprintf (d_status, LCD_DISP_LENGTH + 1, content, ap);
  va_end (ap);
  d_status_ch = 1;
}

void
d_content_update (char *content, ...)
{
  va_list ap;

  va_start (ap, content);
  vsnprintf (d_content, LCD_DISP_LENGTH + 1, content, ap);
  va_end (ap);
  d_content_ch = 1;
}

void
fetch (void)
{
  uint8_t i;

  if (have_ts)
    {
      for (i = 0; i <= t_sensors_count - 1; i++)
	t_val[i] = (float) gtemp (i) / 10;
    }
  for (i = 0; i <= H_SENSORS - 1; i++)
    h_val[i] = get_humidity (i);
  if (have_ac)
    {
      accel[X] = lis_rxa ();
      accel[Y] = lis_rya ();
      accel[Z] = lis_rza ();
    }
}

int
main (void)
{
  uint8_t choice = 0;
  char temp_format[] = "T %d: %2.1f C";
  char humid_format[] = "Fi %d: %d %";
  char accel_format[] = "XYZ %d:%d:%d";

  int16_t keep_temp = 18;
  
  configure ();
    
  /* not yet finished */
 mainloop:
  fetch ();
  if (button_0)
    {
      choice++;
      button_0 = 0;
    }
  switch (choice)
    {
    case 0:
    case 1:
      {
	d_status_update ("Temperature");
	d_content_update (temp_format, choice + 1, (double) t_val[choice]);
	break;
      }
    case 2:
      {
	d_status_update ("Humidity");
	d_content_update (humid_format, choice - 1, h_val[choice - 2]);
	break;
      }
    case 3:
      {
	d_status_update ("Acceleration");
	d_content_update (accel_format, accel[X], accel[Y], accel[Z]);
	break;
      }
    case 4:
      {
	if (button_1)
	  {
	    keep_temp++;
	    button_1 = 0;
	  }
	d_status_update ("Temp.: %d", (int) t_val[TEMP_REG_SENSOR]);
	d_content_update ("%d", keep_temp);
	break;
      }
    case 5:
      goto taploop;
    case 6:
      {
	choice = 0;
	break;
      }

    }
  if ((int) t_val[TEMP_REG_SENSOR] < keep_temp)
    {
      TEMP_REG_PORT |= TEMP_REG_PIN;
    }
  else
    {
      TEMP_REG_PORT &= ~TEMP_REG_PIN;
    }
  d_update ();
  _delay_ms (30);
  goto mainloop;

  /* experimental vibration detection. yay! */
 taploop:
  d_status_update ("XYZ %d %d %d", lis_rxa (), lis_rya (), lis_rza ());
  if (lis_rza () != 0)
    {
      _delay_ms (18);
      if (lis_rza () != 0)
  	{
  	  accel_taps++;
  	  d_content_update ("%d", accel_taps);
  	  d_update ();
  	}
    }
  if (button_0)
    {
      button_0 = 0;
      choice++;
      goto mainloop;
    }
  _delay_ms (50);
  goto taploop;
}

/*
  message format:
  0 byte: [0: temperature-present][1: accelerometer-present][2-7: number of t. sensors]
  [temperature data] <int16_t>
  [humidity data] <uin8_t> <number of h.sensors is assumed 1 for now>
  //[acceleration values (3)] <int16_t> - not transmitted so far.
*/
usbMsgLen_t
usbFunctionSetup (unsigned char setupData[8])
{
  uint8_t i;
  /* buf *shouldn't* be any greater than that. may need fixing anyway. */
  volatile unsigned char zbyte = 0, buf[128], *p;
  volatile int16_t *ptr;

  zbyte = have_ts;
  zbyte |= have_ac << 1;
  zbyte |= t_sensors_count << 2;
  buf[0] = zbyte;

  ptr = (int16_t *) (buf + 1);
  for (i = 0; i < t_sensors_count; i++)
    {
      *ptr = (int16_t) (t_val[i] * 10);
      ptr++;
    }
  /* there's just one sensor anyway */
  p = (unsigned char *) ptr;
  *p = (uint8_t) h_val[0];
  p++;
  usbMsgPtr = buf;
  
  return p - buf;
}
