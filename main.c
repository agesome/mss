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

char disp[32], line[16], usb[16], *usbPtr = &disp[16];
unsigned int nPos = 0, nFunc = 0, hour = 12, min = 0, sec = 0, usbCount = 0;
unsigned int swDelay0 = 1, swDelay1 = 1, swTemp = 25;

void init(void){
	MCUCR = _BV(ISC11); //interrupt sense control
	GICR = _BV(INT1) | _BV(INT2); //interrupts
	PORTD = _BV(PD3); PORTB = _BV(PB2);
	
	search_sensors();
	adc_setup();
	lcd_init(LCD_DISP_ON_BLINK);
	
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
	if(!swDelay1){
	swDelay1 = 1;
	}
}

ISR(INT1_vect, ISR_NOBLOCK ){
	if(!swDelay0){
	swDelay0 = 1;
	}
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
	if(hour == 24){
		hour = 0;
	}
}

ISR(TIMER2_OVF_vect, ISR_NOBLOCK ){
	usbCount++;
	if(usbCount > 2){
	usbPoll();
	usbCount = 0;
	}
}

usbMsgLen_t usbFunctionSetup(unsigned char setupData[8]){
    usbRequest_t *rq = (void *)setupData;
	usbMsgPtr = usb;
	return 32;
}

int main(void){
	
	char *stext[] = {"T 1, C: %2.1f", "T 2, C: %2.1f", "T 3, C: %2.1f", 
	"Fi 1, %%: %2.1f", "Fi 2, %%: %2.1f", "Set T1: %d", "Set time", "%.2d:%.2d \n"};
	int Temp, Hmd;
	init();

	while(1){
	Temp = gtemp(0);
	Hmd = mhumid(0);
	sprintf(usb, "T: %2.1f ; H: %2.1f", (double)gtemp(0)/10, (double)mhumid(0)/10);

	/*process buttons*/
	if(swDelay1){
		nPos++;
		swDelay1 = 0;
	}
	if(swDelay0){
	switch(nPos){
		case 5:
		swTemp++;
		break;
		case 6:
		hour++;
		break;
		case 7:
		min++;
		break;
	}
	//lcd_putc(0xff);
	swDelay0 = 0;
	}
	
	if(nPos == 8){
		nPos = 0;
	}
	if(swTemp > 125){
		swTemp = 0;
	}

	sprintf(disp, stext[7], hour, min);
	

	if(nPos <= 2)
	sprintf(line, stext[nPos], (float)gtemp(nPos)/10);
	else if(nPos > 2 && nPos <= 4)
	sprintf(line, stext[nPos], (float)mhumid(1)/10);
	else if(nPos == 5)
	sprintf(line, stext[nPos], swTemp);
	else if(nPos > 5)
	sprintf(line, "Set Time");

	//sprintf(line, stext[0], (float)gtemp(0)/10);
	//sprintf(line, stext[nPos], (float)mhumid(3)/10);

/*
	if(gtemp(0) < swTemp * 10 && !hOn){
	PORTA |= _BV(PA4);
	hOn = 1;
	}
	if(gtemp(0) > swTemp * 10 && hOn){
	PORTA ^= _BV(PA4); 
	hOn = 0;
	} 
*/

	
	strcpy(disp,strcat(disp,line));
	lcd_clrscr();
	lcd_puts(disp);	

	_delay_ms(200);
	}
}
