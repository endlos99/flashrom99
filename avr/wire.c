/*
 * wire.c: low-level signal handling
 */

#include <avr/io.h>
#include <util/delay.h>

#include "wire.h"


/*
 * pins
 *
 * A7  A7         B7  LED       C7  A14       D7  D7        
 * A6  A6         B6  IMG EN    C6  A13       D6  D6        
 * A5  A5         B5  IMG DAT   C5  A12       D5  D5        
 * A4  A4         B4  --        C4  A11       D4  D4        
 * A3  A3         B3  SD MISO   C3  BUS EN    D3  D3        
 * A2  A2         B2  SD MOSI   C2  RAM WE    D2  D2        E2  A10
 * A1  A1         B1  SD SCK    C1  RAM RE    D1  D1        E1  A9
 * A0  A0         B0  SD CS     C0  RAM CE    D0  D0        E0  A8
 */

#define SENDER_BYTES  9   // FILENAME \0


/*
 * isolate FlashROM 99 from TI cart bus
 */

void disableBus()
{
  // take control and set up SRAM
  PORTC =
    (1 << PC3) |  // isolate cart bus
    (1 << PC2) |  // no RAM writes
    (1 << PC1) |  // no RAM reads
    (0 << PC0);   // enable SRAM chip
  DDRC = 0xff;

  // configure outputs
  DDRA = 0xff;
  DDRB = 0x80;  // SD managed by PFF library
  DDRD = 0xff;
  DDRE = 0x07;

  // LED indicator on
  PORTB |= 1 << PB7;  // LED high = bus disabled
}


/*
 * deactivate all AVR outputs and connect SRAM to TI cart bus
 */

void enableBus()
{
  // prepare SRAM for ROM mode
  PORTC |=
    (1 << PC2) |  // ROM mode: no writes
    (1 << PC1) |  // ROMS* drives RAM RE
    (1 << PC0);   // disable RAM during hand-over
  // alternatively, ROMS* drives CE, RE permanently active

  // disable all bus outputs
  DDRA = 0x00;
  DDRB = 0x80;  // LED remains output
  DDRC = 0x09;  // BUS, RAM CE remain output
  DDRD = 0;
  DDRE = 0;

  // tri-state unused pins
  PORTA = 0x00;
  PORTB = 0x00;  // LED indicator off
  PORTC &= 0x0f;
  PORTD = 0x00;
  PORTE = 0x00;
  
  // release control
  PORTC &= ~((1 << PC3) |
	     (1 << PC0));  // SRAM permanently enabled
}


/*
 * set SRAM address and data
 */

void setRAM(uint16_t addr, uint8_t byte)
{
  // RAM WE still high

  // address: (MSB)  X C7 ..... C4 E2 .. E0 A7 ................. A0 (LSB)
  //                15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
  PORTC = (PORTC & 0x0f) | ((addr & 0x7800) >> 7);  // A14 .. A11
  PORTE = (addr & 0x0700) >> 8;  // A10 .. A8
  PORTA = addr & 0x00ff;  // A7 .. A0

  PORTD = byte;  // data

  PORTC &= ~(1 << PC2);  // RAM WE low
  _delay_us(1);
  PORTC |= 1 << PC2;  // RAM WE high
}


/*
 * read TI data via serial connection (MSB to LSB)
 * - next/done:  CLR @>6000
 * - send 0:     CLR @>7000
 * - send 1:     CLR @>7800
 */

void receiveData(uint8_t *buffer)
{
  uint8_t b;

  // get to known state (IMGEN low), initial CLR @>6000 comes too late
  while (PINB & (1 << PINB6));

  // read serial data from TI
  for (uint8_t i = 0; i < SENDER_BYTES; ++i) {
    b = 0;
    for (uint8_t j = 0; j < 8; ++j) {
      // wait for bit (IMGEN high)
      while (!(PINB & (1 << PINB6)));
      _delay_us(2);
      // read bit
      b = (b << 1) + (PINB & (1 << PINB5) ? 1 : 0);
      // wait until IMGEN low again
      while (PINB & (1 << PINB6));
    }
    *buffer++ = b;
  }
}


/*
 * fatal error: blink LED
 */

void flash_error(const uint8_t count)
{
  while(1) {
    for (uint8_t i = 0; i < count; ++i) {
      PORTB |= 1 << PB7;
      _delay_ms(300);
      PORTB &= ~(1 << PB7);
      _delay_ms(300);
    }
    _delay_ms(500);
  }
}


/* --- debugging -------------------------------------------------- */

#ifdef DEBUG

/*
 *  diagnostic output by attaching 7 segment display to pins Ax
 *  segment a = A6 ... g = A0
 */

const uint8_t segment[] = {
  //  abcdefg
  0b1111110,  // 0
  0b0110000,
  0b1101101,
  0b1111001,
  0b0110011,
  0b1011011,
  0b1011111,
  0b1110000,
  0b1111111,
  0b1111011,  // 9
  0b1110111,  // a
  0b0011111,
  0b1001110,
  0b0111101,
  0b1001111,
  0b1000111,  // f
  0b0000000   // all off
};


/*
 *  display byte nybble by nybble
 */

void show(uint8_t n)
{
  PORTA = segment[n >> 4];
  _delay_ms(700);
  PORTA = segment[n & 0xf];
  _delay_ms(700);
  PORTA = 0;
  _delay_ms(900);
}

#endif
