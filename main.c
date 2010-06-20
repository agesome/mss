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
#define BUTTON_CLICK_DELAY 20

#define H_SENSORS 1
#define T_MAXSENSORS 3

#define ACCEL_DATA_SIZE 4

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
/* sensor presence indication */
static uint8_t have_ts = 0, have_ac = 0;
/* acceleration data */
static int16_t acceleration_data[ACCEL_DATA_SIZE][3], latest_accel[3];
static uint8_t acceleration_data_cell = 0;
/* static int16_t *latest_accel_data = NULL; */
static uint16_t t_delay = 0;

void d_update (void);
void d_status_update (char *content, ...);
void d_content_update (char *content, ...);

void
configure (void)
{
  /* usb stuff. make the host re-enumerate our device */
  cli ();
  usbDeviceDisconnect ();
  /* configure timer 2 for calling usbPoll() */
  TIMSK2 = _BV (TOIE2);         /* overflow interrupt enabled */
  TCCR2B = _BV (CS02) | _BV (CS01) | _BV (CS00);        /* set prescaler to 1024, timer starts */
  /* end of usb stuff  */

  /* display configuration */
  d_status = &display[0];
  display[LCD_DISP_LENGTH] = '\0';
  d_content = &display[LCD_DISP_LENGTH + 1];
  display[LCD_DISP_LENGTH * 2 + 1] = '\0';
  lcd_init (LCD_DISP_ON);
  /* end display configuration */
  /* temperature sensors */
  t_sensors_count = search_sensors ();
  if (t_sensors_count)
    {
      have_ts = 1;
      d_status_update ("Found %d DS18B20", t_sensors_count);
      d_update ();
    }
  /* end if temperature sensors setup */
  /* twi/accelerometer configuration */
  i2c_init ();
  /* don't hurry! */
  _delay_ms (1000);
  if (!lis_initialize (0, 1, 0, 1))
    {
      have_ac = 1;
      d_content_update ("Found LIS302DL");
      d_update ();
    }
  /* end of accelerometer configuration */
  _delay_ms (1500);
  /* configure timer 0 for button state detection */
  TIMSK0 = _BV (TOIE0);         /* enable overflow interrupt */
  TCCR0B = _BV (CS02) | _BV (CS00);     /* set prescaler to 1024, timer starts */
  PORTD |= _BV (PD3) | _BV (PD4);       /* pullup for buttons */
  /* end if button detection setup */
  /* ADC configuration goes here */
  ADMUX = _BV (REFS0);
  ADCSRA = _BV (ADEN) | _BV (ADPS2) | _BV (ADPS1) | _BV (ADPS0);
  PORTC |= _BV (PC0) | _BV (PC1);
  /* end of ADC configuration */

  /* clear the display */
  d_content_update (" ");
  d_status_update (" ");
  d_update ();

  usbDeviceConnect ();
  usbInit ();
  sei ();
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
  if (usb_delay == 1)
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
      if (t_delay++ > 4)
        {
          for (i = 0; i <= t_sensors_count - 1; i++)
            t_val[i] = (float) read_meas (i) / 10;
          t_delay = 0;
        }
      else
        {
          start_meas ();
        }
    }
  for (i = 0; i <= H_SENSORS - 1; i++)
    h_val[i] = get_humidity (i);
  if (have_ac)
    {
      if (acceleration_data_cell < ACCEL_DATA_SIZE)
        {
          latest_accel[X] = acceleration_data[acceleration_data_cell][X] =
            lis_rxa ();
          latest_accel[Y] = acceleration_data[acceleration_data_cell][Y] =
            lis_rya ();
          latest_accel[Z] = acceleration_data[acceleration_data_cell][Z] =
            lis_rza ();
          acceleration_data_cell++;
        }
      else
        {
          latest_accel[X] = lis_rxa ();
          latest_accel[Y] = lis_rya ();
          latest_accel[Z] = lis_rza ();
        }
    }
}

int
main (void)
{
  uint8_t choice = 0;

  int16_t keep_temp = 18;

  configure ();

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
        d_content_update ("T %d: %2.1f C", choice + 1,
                          (double) t_val[choice]);
        break;
      }
    case 2:
      {
        d_status_update ("Humidity");
        d_content_update ("Fi %d: %d %", choice - 1, h_val[choice - 2]);
        break;
      }
    case 3:
      {
        if (button_1)
          {
            keep_temp++;
            button_1 = 0;
          }
        d_status_update ("Temp.: %d", (int) t_val[TEMP_REG_SENSOR]);
        d_content_update ("Keep temp: %d", keep_temp);
        break;
      }
    case 4:
      d_status_update ("X:Y:Z, g");
      d_content_update ("%2.2f %2.2f %2.2f",
                        (float) abs (latest_accel[X]) / 1000,
                        (float) abs (latest_accel[Y]) / 1000,
                        (float) abs (latest_accel[Z]) / 1000);
      break;
    case 5:
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
  _delay_ms (200);
  goto mainloop;
}

/*
  message format:
  0 byte: [0: temperature-present][1: accelerometer-present][2-7: number of t. sensors]
  [temperature data] <int16_t>
  [humidity data] <uin8_t> <number of h.sensors is assumed 1 for now>
  [acceleration values (3)] <int16_t> - not transmitted so far.
*/
USB_PUBLIC usbMsgLen_t
usbFunctionSetup (unsigned char setupData[8])
{
  uint8_t i, j;
  /* buf *shouldn't* be any greater than that. may need fixing anyway. */
  volatile unsigned char buf[64], *p;
  volatile int16_t *ptr;

  buf[0] = have_ts;
  buf[0] |= have_ac << 1;
  buf[0] |= t_sensors_count << 2;

  ptr = (int16_t *) (buf + 1);
  for (i = 0; i < t_sensors_count; i++)
    {
      *ptr = (int16_t) (t_val[i] * 10);
      ptr++;
    }
  for (i = 0; i < acceleration_data_cell; i++)
    {
      for (j = 0; j < 3; j++)
        {
          *ptr = (int16_t) (acceleration_data[i][j]);
          ptr++;
        }
    }
  acceleration_data_cell = 0;
  /* there's just one sensor anyway */
  p = (unsigned char *) ptr;
  *p = (uint8_t) h_val[0];
  p++;

  usbMsgPtr = (unsigned char *) buf;
  return p - buf;
}
