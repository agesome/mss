/*************************************************************************
* Title:    I2C master library using hardware TWI interface
* Author:   Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
* File:     $Id: twimaster.c,v 1.3 2005/07/02 11:14:21 Peter Exp $
* Software: AVR-GCC 3.4.3 / avr-libc 1.2.3
* Target:   any AVR device with hardware TWI 
* Usage:    API compatible with I2C Software Library i2cmaster.h
**************************************************************************/  
#include <inttypes.h>
#include <compat/twi.h>
  
#include <i2cmaster.h>
  
/* define CPU frequency in Mhz here if not defined in Makefile */ 
#ifndef F_CPU
#define F_CPU 4000000UL
#endif	/* 
  
/* I2C clock in Hz */ 
#define SCL_CLOCK  100000L
  
/*************************************************************************
 Initialization of the I2C bus interface. Need to be called only once
*************************************************************************/ 
  void
i2c_init (void) 
{
  
    /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */ 
    
  


  
/*************************************************************************	
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1= failed to access device
*************************************************************************/ 
  unsigned char
i2c_start (unsigned char address) 
{
  
  
    // send START condition
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  
    // wait until transmission completed
    while (!(TWCR & (1 << TWINT)));
  
    // check value of TWI Status Register. Mask prescaler bits.
    twst = TW_STATUS & 0xF8;
  
    return 1;
  
    // send device address
    TWDR = address;
  
  
    // wail until transmission completed and ACK/NACK has been received
    while (!(TWCR & (1 << TWINT)));
  
    // check value of TWI Status Register. Mask prescaler bits.
    twst = TW_STATUS & 0xF8;
  
    return 1;
  



/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready
 
 Input:   address and transfer direction of I2C device
*************************************************************************/ 
  void
i2c_start_wait (unsigned char address) 
{
  
  
    
    {
      
	// send START condition
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
      
	// wait until transmission completed
	while (!(TWCR & (1 << TWINT)));
      
	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
      
	continue;
      
	// send device address
	TWDR = address;
      
      
	// wail until transmission completed
	while (!(TWCR & (1 << TWINT)));
      
	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
      
	
	{
	  
	    /* device busy, send stop condition to terminate write operation */ 
	    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	  
	    // wait until stop condition is executed and bus released
	    while (TWCR & (1 << TWSTO));
	  
	
      
	//if( twst != TW_MT_SLA_ACK) return 1;
	break;
    



/*************************************************************************
 Issues a repeated start condition and sends address and transfer direction 

 Input:   address and transfer direction of I2C device
 
 Return:  0 device accessible
          1 failed to access device
*************************************************************************/ 
  unsigned char
i2c_rep_start (unsigned char address) 
{
  



/*************************************************************************
 Terminates the data transfer and releases the I2C bus
*************************************************************************/ 
  void
i2c_stop (void) 
{
  
    /* send stop condition */ 
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
  
    // wait until stop condition is executed and bus released
    while (TWCR & (1 << TWSTO));



/*************************************************************************
  Send one byte to I2C device
  
  Input:    byte to be transfered
  Return:   0 write successful 
            1 write failed
*************************************************************************/ 
  unsigned char
i2c_write (unsigned char data) 
{
  
  
    // send data to the previously addressed device
    TWDR = data;
  
  
    // wait until transmission completed
    while (!(TWCR & (1 << TWINT)));
  
    // check value of TWI Status Register. Mask prescaler bits
    twst = TW_STATUS & 0xF8;
  
    return 1;
  



/*************************************************************************
 Read one byte from the I2C device, request more data from device 
 
 Return:  byte read from I2C device
*************************************************************************/ 
  unsigned char
i2c_readAck (void) 
{
  
  
  



/*************************************************************************
 Read one byte from the I2C device, read is followed by a stop condition 
 
 Return:  byte read from I2C device
*************************************************************************/ 
  unsigned char
i2c_readNak (void) 
{
  
  
  


