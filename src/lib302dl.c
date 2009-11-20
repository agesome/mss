/* i2c library compatibility functions */
#include <stdio.h>
#include <avr/io.h>
#include "lis_compat.h"
#include "lis_defines.h"

/* write register */
void
lis_rwrite (uint8_t reg, uint8_t value)
{
  twi_start (LIS_WRITE_ADDR);
  twi_write (reg);
  twi_write (value);
  twi_stop ();
}

int8_t
lis_rread (uint8_t reg)
{
  int8_t recv;

  twi_start (LIS_WRITE_ADDR);
  twi_write (reg);
  twi_stop ();
  twi_start (LIS_READ_ADDR);
  recv = twi_read ();
  twi_stop ();

  return recv;
}

uint8_t
lis_initialize (uint8_t high_datarate, uint8_t dopowerup, uint8_t setfullscale)
{
  if (lis_rread (LIS_WHOAMI) != LIS_WHOAMI_VALUE)
    return 1;

  if (dopowerup)
    lis_rwrite (LIS_CR1, _BV(LIS_PD));
  if (high_datarate)
    lis_rwrite (LIS_CR1, _BV(LIS_DR));
  if (setfullscale)
    lis_rwrite (LIS_CR1, _BV(LIS_FS));

  return 0;
}

int8_t
lis_rx (void)
{
  return lis_rread (LIS_OX);
}

int8_t
lis_ry (void)
{
  return lis_rread (LIS_OY);
}

int8_t
lis_rz (void)
{
  return lis_rread (LIS_OZ);
}
