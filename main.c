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

#define USB_REQ_LEN 64
#define TEMP_REG_SENSOR 0
#define TEMP_REG_PORT PORTA
#define TEMP_REG_PIN PA4
/* A == port, B == pin */
#define ISUP(A, B) !(A & (~A | _BV(B)))

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

static char disp[LCD_DISP_LENGTH * 2], line[LCD_DISP_LENGTH];
char usbBuff[USB_REQ_LEN];
static unsigned short nPos = 0, hour = 12, min = 0, sec = 0,
  usbCount = 0, swDelay0 = 0, swDelay1 = 0, swd1 = 0, swd0 = 0,
  nSensors = 0, dispUpdate = 0, sD = 0;
static double tData[MAXTS], hData[MAXHS];

void updateScr(void);

void
setup(void){
	nSensors = search_sensors();
	lcd_init(LCD_DISP_ON);
	
	TIMSK = _BV(OCIE1A) | _BV(TOIE2) | _BV(TOIE0);
	OCR1A = 46875; //one second
	
	/* re-enumerate */
	usbDeviceDisconnect();
	_delay_ms(100);
	usbDeviceConnect();
	usbInit();

	sei();
	/*timers on*/
	TCCR1B = _BV(WGM12) | _BV(CS12);
	TCCR2 = _BV(CS22) | _BV(CS21) | _BV(CS20);
	TCCR0 = _BV(CS02) |  _BV(CS00);
}

ISR(TIMER1_COMPA_vect, ISR_NOBLOCK ){
	sec++;
	if(sec >= 60){
	  sec = 0;
	  min++;
	}
	if(min >= 60){
	  min = 0;
	  hour++;
	}
	if(hour >= 24)
	  hour = 0;
}

ISR(TIMER2_OVF_vect, ISR_BLOCK){
  usbPoll();
}

ISR(TIMER0_OVF_vect, ISR_NOBLOCK ){
  /*check for buttons activity*/
  if(ISUP(PINB, PB2) && !swDelay1){
	if(swd1){
	  swDelay1 = 1;
	  swd1 = 0;
	}
	else
	  swd1 = 1;
  }
  if(ISUP(PIND, PD3) && !swDelay0){
	if(swd0){
	  swDelay0 = 1;
	  swd0 = 0;
	}
	else
	  swd0 = 1;
  }
  dispUpdate++;
  if(dispUpdate >= 15 && !sD){
	updateScr();
	dispUpdate = 0;
  }
}

usbMsgLen_t
usbFunctionSetup(unsigned char setupData[8]){
  usbRequest_t *rq = (void *)setupData;
  sprintf(usbBuff, "%2.1f %2.1f %2.1f %2.1f %2.1f", tData[0], tData[1], tData[2], hData[0], hData[1]);
  usbMsgPtr = (unsigned char *)usbBuff;
  return USB_REQ_LEN;
}

void
fillData(double tData[], double hData[]){
  unsigned short i;
  if(tData != NULL)
  for(i = 0; i < nSensors; i++){
    tData[i] = (double)gtemp(i);
    tData[i] /= 10;
  }
  if(hData != NULL){
    hData[0] = (double)mhumid(0);
    hData[0] /= 10;
    hData[1] = (double)mhumid(2);
    hData[1] /= 10;
  }
}

void
sensorShow(void){
  lcd_clrscr();
  sprintf(disp, "Sensor detection...\n");
  lcd_puts(disp);
  while(!swDelay0){
    double dtData[MAXTS];
    unsigned short i;
    fillData(tData, NULL);
    _delay_ms(150);
    fillData(dtData, NULL);
    for(i = 0; i < nSensors; i++){
      if(tData[i] < dtData[i]){
	sprintf(line, "Sensor %d up!", i);
	lcd_clrscr();
	lcd_puts(line);
      }
      else if(tData[i] > dtData[i]){
	sprintf(line, "Sensor %d down!", i);
	lcd_clrscr();
	lcd_puts(line);
      }
    }
  }
}

void
updateScr(void){
  lcd_clrscr();
  lcd_puts(disp);
  lcd_puts(line);
}

void
main(void){
  const char *stext[] = {"T1, C: %2.1f", "T 2, C: %2.1f", "T 3, C: %2.1f", 
			 "Fi 1, %%: %2.1f", "Fi 2, %%: %2.1f", "Set temp.: %d", "Set time", "%.2d:%.2d \n"};
  unsigned short hOn = 0, swTemp = 25;
  
  setup();
  
  loop:
    fillData(tData, hData);
/*     process buttons */
    if(nPos < 5 && swDelay0){
      swDelay0 = 0;
	  sD = 1;
      sensorShow();
	  sD = 0;
    }
    
    if(swDelay1){
      nPos++;
      swDelay1 = 0;
    }
    if(nPos > 7)
      nPos = 0;
    if(swDelay0){
      switch(nPos - 4){
      case 1:
	swTemp++;
	break;
      case 2:
	hour++;
	break;
      case 3:
	min++;
	break;
      }
      swDelay0 = 0;
    }

	if(!sD){
    sprintf(disp, stext[7], hour, min, sec);
    
    switch(nPos){
    case 0: case 1: case 2:
      sprintf(line, stext[nPos], tData[nPos]);
      break;
    case 3: case 4:
      sprintf(line, stext[nPos], hData[nPos-3]);
      break;
    case 5:
      sprintf(line, stext[nPos], swTemp);
      break;
    case 6:
      sprintf(line, stext[nPos]);
      break;
	}
    }
      
    /*temperature regulation*/
    if(tData[TEMP_REG_SENSOR] < swTemp && !hOn){
      TEMP_REG_PORT |= _BV(TEMP_REG_PIN);
      hOn = 1;
    }
    if(gtemp(0) > swTemp * 10 && hOn){
      TEMP_REG_PORT ^= _BV(TEMP_REG_PIN);
      hOn = 0;
    }
	goto loop;
}

