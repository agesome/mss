void lis_rwrite (uint8_t reg, uint8_t value);
int8_t lis_rread (uint8_t reg);
uint8_t lis_initialize (uint8_t high_datarate, uint8_t dopowerup, uint8_t setfullscale);
int8_t lis_rx (void);
