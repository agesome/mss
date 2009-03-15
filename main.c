/*
 *      main.c
 * 
 * 		Main program.
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
 * 
 */
#define DISP_UPDATE_DELAY 50
#define USB_REQ_LEN 32

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
static unsigned short int nPos = 0, hour = 12, min = 0, sec = 0,
usbCount = 0, swDelay0 = 0, swDelay1 = 0;
static double tData[MAXTS], hData[MAXHS];

void
init(void){
	MCUCR = _BV(ISC11); //interrupt sense control
	GICR = _BV(INT1) | _BV(INT2); //interrupts
	PORTD = _BV(PD3); PORTB = _BV(PB2);
	
	search_sensors();
	adc_setup();
	lcd_init(LCD_DISP_ON);
	
	TIMSK = _BV(OCIE1A) | _BV(TOIE2);
	OCR1A = 46875; //one second
	
	usbDeviceDisconnect();
	_delay_ms(100);
	usbDeviceConnect();
	usbInit();

	sei();
	/*timers on*/
	TCCR1B = _BV(WGM12) | _BV(CS12);
	TCCR2 = _BV(CS22) | _BV(CS21);
}

ISR(INT2_vect, ISR_NOBLOCK ){
	if(!swDelay1)
	  swDelay1 = 1;
}

ISR(INT1_vect, ISR_NOBLOCK ){
	if(!swDelay0)
	  swDelay0 = 1;
}

ISR(TIMER1_COMPA_vect, ISR_NOBLOCK ){
	sec++;
	if(sec == 60){
	  sec = 0;
	  min++;
	}
	if(min == 60){
	  min = 0;
	  hour++;
	}
	if(hour == 24)
	  hour = 0;
}

ISR(TIMER2_OVF_vect, ISR_NOBLOCK ){
	usbCount++;
	if(usbCount > 2){
	  usbPoll();
	  usbCount = 0;
	}
}

usbMsgLen_t
usbFunctionSetup(unsigned char setupData[8]){
    usbRequest_t *rq = (void *)setupData;
    
    /*
      0: send temperature value and humidity percentage;
      1: send all data from eeprom and erease it;
      2: do 1 but do not erease data;
    */
    switch(rq->bRequest){
    case 2:
      sprintf(usbBuff, "%2.1f %2.1f %2.1f %2.1f %2.1f", tData[0], tData[1], tData[2], hData[0], hData[1]);
      usbMsgPtr = (unsigned char *)usbBuff;
      return strlen(usbBuff + 1);
	break;
    }
    return 0;
}

int
main(void){
  const char *stext[] = {"T 1, C: %2.1f", "T 2, C: %2.1f", "T 3, C: %2.1f", 
			 "Fi 1, %%: %2.1f", "Fi 2, %%: %2.1f", "Set time", "%.2d:%.2d \n"};
  int i;
  
  init();
  
  while(1){
    /* quite ugly, needs a fix */
    for(i = 0; i <= MAXTS; i++){
      tData[i] = (double)gtemp(i);
      tData[i] /= 10;
    }
    for(i = 0; i <= MAXHS; i++){
      hData[i] = (double)mhumid(i);
      hData[i] /= 10;
    }
    /*process buttons*/
    if(swDelay1){
      nPos++;
      swDelay1 = 0;
    }
    if(nPos > 6)
      nPos = 0;
    if(swDelay0){
      switch(nPos - 4){
      case 0:
	hour++;
	break;
      case 1:
	min++;
	break;
      }
      swDelay0 = 0;
    }
    
    sprintf(disp, stext[6], hour, min);
    
    if(nPos <= 2)
      sprintf(line, stext[nPos], tData[nPos]);
    else if(nPos >= 3 && nPos <= 4)
      sprintf(line, stext[nPos], hData[nPos]);
    else if(nPos >= 6)
      sprintf(line, "Set Time");
    
    strcpy(disp,strcat(disp,line));
    lcd_clrscr();
    lcd_puts(disp);
    _delay_ms(DISP_UPDATE_DELAY);
  }
}
