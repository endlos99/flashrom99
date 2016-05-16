// mock types for avr

// stdint
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

// pgmspace.h

#define PROGMEM
uint8_t pgm_read_byte(const uint8_t *p);
