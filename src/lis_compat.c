/* compatibility functions; library intended to be used with Peter Fleury's i2c library, */
/* but should be useably with anything else */
#include <stdio.h>
#include "i2cmaster.h"

void
twi_initialize (void)
{
  i2c_init ();
}

void
twi_stop (void)
{
  i2c_stop ();
}

void
twi_start (uint8_t addr)
{
  i2c_start_wait (addr);
}

uint8_t
twi_write (uint8_t data)
{
  return i2c_write (data);
}

uint8_t
twi_read (void)
{
  return i2c_readNak ();
}
