#include <inttypes.h>

/*******************************************/
/* Hardware connection                     */
/*******************************************/

/* Define OW_ONE_BUS if only one 1-Wire-Bus is used
   in the application -> shorter code.
   If not defined make sure to call ow_set_bus() before using 
   a bus. Runtime bus-select increases code size by around 300 
   bytes so use OW_ONE_BUS if possible */

/* Are set in main file*/
#define OW_PIN  PD5
#define OW_IN   PIND
#define OW_OUT  PORTD
#define OW_DDR  DDRD
#define OW_CONF_DELAYOFFSET 0

/*******************************************/

// #define OW_SHORT_CIRCUIT  0x01

#define OW_MATCH_ROM	0x55
#define OW_SKIP_ROM	    0xCC
#define	OW_SEARCH_ROM	0xF0

#define	OW_SEARCH_FIRST	0xFF	// start new search
#define	OW_PRESENCE_ERR	0xFF
#define	OW_DATA_ERR	    0xFE
#define OW_LAST_DEVICE	0x00	// last device found
//                      0x01 ... 0x40: continue searching

// rom-code size including CRC
#define OW_ROMCODE_SIZE 8
extern uint8_t ow_reset (void);
extern uint8_t ow_bit_io (uint8_t b);
extern uint8_t ow_byte_wr (uint8_t b);
extern uint8_t ow_byte_rd (void);
extern uint8_t ow_rom_search (uint8_t diff, uint8_t * id);
extern void ow_command (uint8_t command, uint8_t * id);
extern void ow_parasite_enable (void);
extern void ow_parasite_disable (void);
extern uint8_t ow_input_pin_state (void);
