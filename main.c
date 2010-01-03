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

/*sensor to regulate temperature according to*/
#define TEMP_REG_SENSOR 0
#define TEMP_REG_PORT PORTA
#define TEMP_REG_PIN PA4

/* buttons are on same port, that simplifies things. */
#define BUTTON_PIN PIND
#define BUTTON0_BIT PD3
#define BUTTON1_BIT PD4
#define BUTTON_CLICK_DELAY 35

#define VCC 5.05
#define H_SENSORS 1
#define B_PRESS_DELAY 13

/* A is the port, B is the pin to check */
#define ISCLEAR(A, B) !(A & (~A ^ _BV(B)) )

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <lcd.h>
#include <humidity.h>
#include <usbdrv.h>
#include <temperature.h>
#include <i2cmaster.h>
#include <lib302dl.h>

/* display buffer */
static char display[LCD_DISP_LENGTH * LCD_LINES + 2];
/* pointers to lines, they're set in configure () */
static char *d_status, *d_content;
/* change indicators, so display is not constantly updated */
static uint8_t d_status_ch = 0, d_content_ch = 0;
/* delay between usbPoll () calls */
static uint16_t usb_delay = 0;
/* usb data buffer */
static unsigned char usbbuf[64];
/* number of detected temperature sensors, set in configure () */
static uint8_t t_sensors_count = 0;
/* temperature and humidity data */
/* number of sensors is unknown at compile time, so just pick some. */
static double t_val[H_SENSORS], h_val[3]; 
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
    d_content_update ("%d found.", t_sensors_count);
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
  if (lis_initialize (0, 1, 0))
    d_content_update ("not found.");
  else
    d_content_update ("found.");
  d_update ();
  _delay_ms (1000);
  /* end of accelerometer configuration */

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
   <signal>      -------------
   <button state>-___-___-___-__________
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

  for (i = 0; i <= t_sensors_count - 1; i++)
    t_val[i] = gtemp (i) / 10;
  for (i = 0; i <= H_SENSORS - 1; i++)
    h_val[i] = mhumid (i) / 10;
  accel[X] = lis_rx ();
  accel[Y] = lis_ry ();
  accel[Z] = lis_rz ();
}

int
main (void)
{
  uint8_t choice = 0, choice1 = 0;
  char temp_format[] = "T %d: %2.1f C";
  char humid_format[] = "Fi %d: %2.1f %";
  char accel_format[] = "XYZ %d:%d:%d";

  configure ();

  /* for debugging needs */
 /* tmploop: */
 /*  if (button_0) */
 /*    { */
 /*      choice++; */
 /*      button_0 = 0; */
 /*    } */
 /*  if (button_1) */
 /*    { */
 /*      choice1++; */
 /*      button_1 = 0; */
 /*    } */
 /*  d_status_update ("%d %d", choice, choice1); */
 /*  d_update (); */
 /*  goto tmploop; */
  
  /* not yet finished */
 mainloop:
  if (button_0)
    {
      choice++;
      button_0 = 0;
    }
  /* d_status_update ("%d", choice); */
  fetch ();
  switch (choice)
    {
    case 0:
    case 1:
      {
	d_status_update ("Temperature");
	d_content_update (temp_format, 1, (double) t_val[0]);
	break;
      }
    case 2:
      {
	d_status_update ("Humidity");
	d_content_update (humid_format, choice - 2, h_val[choice] - 2);
	break;
      }
    case 3:
      {
	d_status_update ("Acceleration");
	d_status_update (accel_format, accel[X], accel[Y], accel[Z]);
	break;
      }
    case 4:
      {
	choice = 0;
	break;
      }
    }
  d_update ();
  _delay_ms (30);
  goto mainloop;
}

usbMsgLen_t
usbFunctionSetup (unsigned char setupData[8])
{
  double t = t_val[0];
  memcpy (usbbuf, (void *) &t, sizeof (t));
  /* memcpy(usbbuf + sizeof(t_val[0]), (void *)&h_val[0], sizeof(h_val[0])); */
  usbbuf[sizeof (t) + 1] = '\0';	//+ sizeof(h_val[0]) + 1] = '\0';
  usbMsgPtr = usbbuf;
  return strlen ((char *) usbbuf);
}
