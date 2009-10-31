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
#define B0_PIN PIND
#define B0_BIT PD3
#define B1_PIN PIND
#define B1_BIT PD4
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

static char display[LCD_DISP_LENGTH * LCD_LINES + 2];
static char *d_status, *d_content;
static uint8_t d_status_ch = 0, d_content_ch = 0, uptime_cnt = 0;
static uint16_t usb_requests = 0, usb_delay = 0;
static uint8_t button_0 = 0, button_1 = 0, sb0, sb1;
/* was-up indicators */
static uint8_t wu0 = 0, wu1 = 0;
static uint8_t ns = 0;
static int16_t s0 = 0;
static uint16_t uptime = 0;
static unsigned char usbbuf[64];

void d_update (void);
void d_status_update(char *content, ...);
void d_content_update(char *content, ...);

void
configure (void)
{
  /* usb stuff. make the host re-enumerate out device */
  usbDeviceDisconnect ();
  _delay_ms (100);
  usbDeviceConnect ();
  usbInit ();
  sei();
  /* end of usb stuff  */
  
  /* configure timer 2 for calling usbPoll() */
  TIMSK2 = _BV(TOIE2);			       /* overflow interrupt enabled */
  TCCR2B =  _BV(CS02) | _BV(CS01) | _BV(CS00); /* set prescaler to 1024, timer starts */
  
  /* display configuration */
  d_status = &display[0];
  display[LCD_DISP_LENGTH] = '\0';
  d_content = &display[LCD_DISP_LENGTH + 1];
  display[LCD_DISP_LENGTH * 2 + 1] = '\0';
  
  lcd_init (LCD_DISP_ON);
  /* end display configuration */

  /* temperature sensors */
  ns = search_sensors ();
  if (ns)
    {
      d_status_update ("Temp. sensors:");
      d_content_update ("%d found.", ns);
    }
  else
    {
      d_status_update ("Temp. sensors:");
      d_content_update ("not found.");
    }
  d_update ();
  _delay_ms (1000);
  
  /* configure timer 0 for button state detection */
  TIMSK0 = _BV(TOIE0);		  /* enable overflow interrupt */
  TCCR0B = _BV(CS02) | _BV(CS00); /* set prescaler to 1024, timer starts */
  PORTD |= _BV(PD3) | _BV(PD4);	  /* pullup for button 0 */

  TIMSK1 = _BV(OCIE1A);
  /* reaching this with a prescaler of 256 and frequency of 20Mhz five times is exactly one second */
  OCR1A = 15625;
  TCCR1B = _BV(CS12) | _BV(WGM12);
}

ISR (TIMER1_COMPA_vect)
{
  sei ();
  
  uptime_cnt++;

  if (uptime_cnt == 5)
    {
      uptime++;
      uptime_cnt = 0;
    }
}

ISR (TIMER0_OVF_vect)
{
  sei ();
  
  /* button 0 */
  if (!button_0)
    {
      if (!wu0)
	{
	  if (ISCLEAR(B0_PIN, B0_BIT))
	    wu0 = 1;
	}
      else
	if (ISCLEAR(B0_PIN, B0_BIT))
	  {
	    button_0 = 1;
	    wu0 = 0;
	    sb0 = 1;
	  }
    }
  else
    {
      if (!ISCLEAR(B0_PIN, B0_BIT) && !sb0)
	button_0 = 0;
    }

  /* button 1 */
  if (!button_1)
    {
      if (!wu1)
	{
	  if (ISCLEAR(B1_PIN, B1_BIT))
	    wu1 = 1;
	}
      else
	if (ISCLEAR(B1_PIN, B1_BIT))
	  {
	    button_1 = 1;
	    wu1 = 1;
	    sb1 = 1;
	  }
    }
  else
    {
      if (!ISCLEAR(B1_PIN, B1_BIT) && !sb1)
	button_1 = 1;
    }  
}

ISR (TIMER2_OVF_vect)
{
  cli ();
  
  usb_delay++;

  /* timer 2 overflows in ~13 msec, so we'll call usbPoll() each 39 msec, which is just fine */
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
d_status_update(char *content, ...)
{
  va_list ap;

  va_start(ap, content);
  vsnprintf(d_status, LCD_DISP_LENGTH + 1, content, ap);
  va_end(ap);
  d_status_ch = 1;
}

void
d_content_update(char *content, ...)
{
  va_list ap;
  
  va_start(ap, content);
  vsnprintf(d_content, LCD_DISP_LENGTH + 1, content, ap);
  va_end(ap);
  d_content_ch = 1;
}

int
main (void)
{
  configure ();
  d_content_update(" ");
  
 mainloop:
  s0 = gtemp (0);
  d_status_update ("uptime: %d sec.", uptime);
  d_content_update ("s0: %d", s0);
  d_update ();
  /* if (button_0) */
  /*   { */
  /*     np++; */
  /*     wu0 = 0; */
  /*     sb0 = 0; */
  /*   } */
  _delay_ms(150);
  goto mainloop;
  
}

usbMsgLen_t
usbFunctionSetup (unsigned char setupData[8])
{
  memcpy(usbbuf, (void *)&s0, sizeof(s0));
  memcpy(usbbuf + sizeof(s0), (void *)&uptime, sizeof(uptime));
  usbbuf[sizeof(s0) + sizeof(uptime) + 1] = '\0';
  usbMsgPtr = usbbuf;
  usb_requests++;
  return strlen ((char *) usbbuf);
}

/*static char disp[LCD_DISP_LENGTH * 2], line[LCD_DISP_LENGTH];*/
/*char usbBuff[USB_REQ_LEN];*/
/*volatile static uint8_t nPos = 0, hour = 12, min = 0, sec = 0,*/
/*  swDelay0 = 0, swDelay1 = 0, swd1 = 0, swd0 = 0,*/
/*   dispUpdate = 0, sD = 0;*/
/*static uint8_t nSensors = 0;*/
/*static double tData[MAXTS], hData[MAXHS];*/

/*void*/
/*setup (void)*/
/*{*/
/*  nSensors = search_sensors ();*/
/*  lcd_init (LCD_DISP_ON);*/

/*  TIMSK = _BV (OCIE1A) | _BV (TOIE2) | _BV (TOIE0);*/
/*  OCR1A = 46875;		//one second*/


/*  */
/*  sei ();*/
  /*timers on */
/*  TCCR1B = _BV (WGM12) | _BV (CS12);*/
/*  TCCR2 = _BV (CS22) | _BV (CS21) | _BV (CS20);*/
/*  TCCR0 = _BV (CS02) | _BV (CS00);*/
/*}*/

/*ISR (TIMER1_COMPA_vect, ISR_NOBLOCK)*/
/*{*/
/*  sec++;*/
/*  if (sec >= 60)*/
/*    {*/
/*      sec = 0;*/
/*      min++;*/
/*    }*/
/*  if (min >= 60)*/
/*    {*/
/*      min = 0;*/
/*      hour++;*/
/*    }*/
/*  if (hour >= 24)*/
/*    hour = 0;*/
/*}*/

/*ISR (TIMER2_OVF_vect, ISR_BLOCK)*/
/*{*/
/*  usbPoll ();*/
/*}*/

/*ISR (TIMER0_OVF_vect, ISR_NOBLOCK)*/
/*{*/
  /*check for buttons activity */
/*  if (ISUP (PINB, PB2) && !swDelay1)*/
/*    {*/
/*      if (swd1)*/
/*	{*/
/*	  swDelay1 = 1;*/
/*	  swd1 = 0;*/
/*	}*/
/*      else*/
/*	swd1 = 1;*/
/*    }*/
/*  if (ISUP (PIND, PD3) && !swDelay0)*/
/*    {*/
/*      if (swd0)*/
/*	{*/
/*	  swDelay0 = 1;*/
/*	  swd0 = 0;*/
/*	}*/
/*      else*/
/*	swd0 = 1;*/
/*    }*/
/*  dispUpdate++;*/
/*  if (dispUpdate >= 15 && !sD)*/
/*    {*/
/*      lcd_clrscr ();*/
/*      lcd_puts (disp);*/
/*      lcd_puts (line);*/
/*      dispUpdate = 0;*/
/*    }*/
/*}*/



/*void*/
/*fillData (double tData[], double hData[])*/
/*{*/
/*  unsigned short i;*/
/*  if (tData != NULL)*/
/*    for (i = 0; i < nSensors; i++)*/
/*      {*/
/*	tData[i] = (double) gtemp (i);*/
/*	tData[i] /= 10;*/
/*      }*/
/*  if (hData != NULL)*/
/*    {*/
/*      hData[0] = (double) mhumid (0);*/
/*      hData[0] /= 10;*/
/*      hData[1] = (double) mhumid (2);*/
/*      hData[1] /= 10;*/
/*    }*/
/*}*/

/*void*/
/*sensorShow (void)*/
/*{*/
/*  lcd_clrscr ();*/
/*  lcd_puts ("Sensor detection...\n");*/
/*  while (!swDelay0)*/
/*    {*/
/*      double dtData[MAXTS];*/
/*      unsigned short i;*/
/*      fillData (tData, NULL);*/
/*      _delay_ms (150);*/
/*      fillData (dtData, NULL);*/
/*      for (i = 0; i < nSensors; i++)*/
/*	{*/
/*	  if (tData[i] < dtData[i])*/
/*	    {*/
/*	      sprintf (line, "Sensor %d up!", i);*/
/*	      lcd_clrscr ();*/
/*	      lcd_puts (line);*/
/*	    }*/
/*	  else if (tData[i] > dtData[i])*/
/*	    {*/
/*	      sprintf (line, "Sensor %d down!", i);*/
/*	      lcd_clrscr ();*/
/*	      lcd_puts (line);*/
/*	    }*/
/*	}*/
/*    }*/
/*}*/

/*void*/
/*main (void)*/
/*{*/
/*  const char *stext[] = { "T1, C: %2.1f", "T 2, C: %2.1f", "T 3, C: %2.1f",*/
/*    "Fi 1, %%: %2.1f", "Fi 2, %%: %2.1f", "Set temp.: %d", "Set time",*/
/*    "%.2d:%.2d \n"*/
/*  };*/
/*  unsigned short hOn = 0, swTemp = 25;*/

/*  setup ();*/

/*loop:*/
/*  fillData (tData, hData);*/
/*     process buttons */
/*  if (nPos < 5 && swDelay0)*/
/*    {*/
/*      swDelay0 = 0;*/
/*      sD = 1;*/
/*      sensorShow ();*/
/*      sD = 0;*/
/*    }*/

/*  if (swDelay1)*/
/*    {*/
/*      nPos++;*/
/*      swDelay1 = 0;*/
/*    }*/
/*  if (nPos > 7)*/
/*    nPos = 0;*/
/*  if (swDelay0)*/
/*    {*/
/*      switch (nPos - 4)*/
/*	{*/
/*	case 1:*/
/*	  swTemp++;*/
/*	  break;*/
/*	case 2:*/
/*	  hour++;*/
/*	  break;*/
/*	case 3:*/
/*	  min++;*/
/*	  break;*/
/*	}*/
/*      swDelay0 = 0;*/
/*    }*/

/*  if (!sD)*/
/*    {*/
/*      sprintf (disp, stext[7], hour, min, sec);*/

/*      switch (nPos)*/
/*	{*/
/*	case 0:*/
/*	case 1:*/
/*	case 2:*/
/*	  sprintf (line, stext[nPos], tData[nPos]);*/
/*	  break;*/
/*	case 3:*/
/*	case 4:*/
/*	  sprintf (line, stext[nPos], hData[nPos - 3]);*/
/*	  break;*/
/*	case 5:*/
/*	  sprintf (line, stext[nPos], swTemp);*/
/*	  break;*/
/*	case 6:*/
/*	  sprintf (line, stext[nPos]);*/
/*	  break;*/
/*	}*/
/*    }*/

  /*temperature regulation */
/*  if (tData[TEMP_REG_SENSOR] < swTemp && !hOn)*/
/*    {*/
/*      TEMP_REG_PORT |= _BV (TEMP_REG_PIN);*/
/*      hOn = 1;*/
/*    }*/
/*  if (gtemp (0) > swTemp * 10 && hOn)*/
/*    {*/
/*      TEMP_REG_PORT ^= _BV (TEMP_REG_PIN);*/
/*      hOn = 0;*/
/*    }*/
/*  goto loop;*/
/*}*/
