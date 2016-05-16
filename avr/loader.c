/*
 * loader.c: image loader and menu creator
 */

#ifdef SIMULATE
# include "mocktypes.h"
#else
# include <avr/pgmspace.h>
#endif

#include <lib/pff.h>
#include "loader.h"
#include "wire.h"


/*
 * Memory layout of generated menu:
 *
 * >6000:          GPL header
 * >6010:          menu item loader entries              (8 x 8 bytes)
 *                 general menu and broweser code
 *                 - end of fixed program -
 * TI menu:        TEXT 'FILENAME\0'                     (8 x 36 bytes)
 *                 BYTE <GPL header version>
 *                 DATA <start address in first bank>
 *                 DATA <next item> or 0             |   ($ + 22)
 *                 DATA <menu item loader addr>      |
 *                 TEXT '<len>ENTRY NAME (19 CHARS)' |
 * browser menu:   TEXT 'FILENAME\0'                     (n x 32 chars)
 *                 BYTE <GPL header version>
 *                 DATA <start address in first bank>
 *                 TEXT 'ENTRY NAME (20 CHARS)'
 */

#define MENU_ENTRY_SIZE     36
#define MENU_ENTRY_OFFSET   12  // start of actual entry
#define MENU_HANDLER_SIZE    8
#define BROWSER_ENTRY_SIZE  32
#define ENTRY_NAME_LEN      20

#define MAX_ENTRIES         (9 * 19)

#define MENU_START      0x0800
#define BROWSER_START   (MENU_START + MENU_ENTRY_SIZE * 8)


/*
 * globals and statics
 */

#include "../ti/menu.c"

const uint8_t browser_entry[] PROGMEM = {
  '*', '*', '*', '*', '*', '*', '*', '*', 0, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x60, 0x10 + MENU_HANDLER_SIZE * 8,
  11, 'F', 'L', 'A', 'S', 'H', 'R', 'O', 'M', ' ', '9', '9'
};

// RAM access
uint16_t ram_addr;  // current SRAM address

// menu information
static uint8_t images_written;
static uint8_t files_written;

// SD card data structures
static FATFS sd_fs;
static DIR sd_dp;
static FILINFO sd_fno;
static FRESULT rc;

// universal buffer
static uint8_t buf_entry[MENU_ENTRY_SIZE + 1];
static uint8_t buffer[256];


// internal functions
static uint8_t addEntries(const char *filename);
static void writeMenuCode(void);
static void writeMenuEntry(void);
static void writeBrowserEntry(void);
static void closeEntries(void);
static void writeMem(uint8_t byte);


/*
 * initialize cart
 */

void init()
{
  images_written = files_written = 0;
}


/*
 * build menu by scanning directory for *.BIN files
 */

uint8_t genMenu()
{
  uint8_t *p;
  uint16_t filecount = 0;

  // shut off TI cart bug
  disableBus();
  
  // open SD card
  rc = pf_mount(&sd_fs);
  if (rc)
    flash_error(1);

  rc = pf_opendir(&sd_dp, "");
  if (rc)
    flash_error(2);

  // add menu and browser code
  writeMenuCode();
  
  while (images_written < MAX_ENTRIES) {
    // get next item in directory
    rc = pf_readdir(&sd_dp, &sd_fno);
    if (rc)
      flash_error(3);
    if (sd_fno.fname[0] == 0)  // end of dir
      break;
    ++filecount;
    if (sd_fno.fattrib & AM_DIR)
      continue;  // skip directories

    // check if file ends in ".BIN"
    p = (uint8_t *)sd_fno.fname;
    while (*++p);
    if (p - (uint8_t *)sd_fno.fname < 5 ||
	*--p != 'N' || *--p != 'I' || *--p != 'B' || *--p != '.')
      continue;
    --p;  // p at last char of filename w/o extension

    // check image size
    if (sd_fno.fsize > 32768)
      continue;

#ifdef MULTI_FILE
    // check for multi-file image
    // - if file ends in C -> check how many banks
    // - if file ends in D, E, F -> check if C exists, then ignore
    uint8_t c;
    if (sd_fno.fsize == 8192 && *p == 'C') {
      for (c = 'D'; c <= 'F'; ++c) {
        *p = c;
        rc = pf_open(sd_fno.fname);
        if (rc)
          break;
      }
      *p = 'C';
    } else if (sd_fno.fsize == 8192 && (*p == 'D' || *p == 'E' || *p == 'F')) {
      c = *p;
      *p = 'C';
      rc = pf_open(sd_fno.fname);
      if (!rc)
        continue;  // current file covered by C file
      *p = c;  // actually not a multi-file image
    }
#endif

    // create entry
    if (addEntries(sd_fno.fname))
      ++files_written;
  }

  // if only one image found, load file directly
  if (files_written == 1)
    return 1;

  // close menu and browser items
  closeEntries();

  // bring cartridge online
  enableBus();

  return 0;
}


/*
 * add entries of current file
 */

static uint8_t addEntries(const char *filename)
{
  UINT bytes_read;
  uint8_t i;
  
  rc = pf_fnoopen(&sd_fno);
  if (rc)
    return 0;

  // skip to relevant metadata
  rc = pf_read(buffer, 8, &bytes_read);
  //uint16_t fpos = 8;  // file position, used by lseek replacement
  if (rc || bytes_read < 8 || buffer[0] != 0xaa)
    return 0;  // not an image
  uint8_t version = buffer[1];  // version in GPL header
  
  uint16_t next = ((uint16_t)buffer[6] << 8) + buffer[7];
  uint8_t entry_added = 0;
  while (next) {
    uint16_t offset = next - 0x6000;
    if (offset > 0x1fe0)
      break;  // corrupt image
    if (images_written >= MAX_ENTRIES)
      break;  // too many images
    
    if (pf_lseek(offset))
      return entry_added;

#if 0  // smaller, but potentially slower replacement for lseek
    // read ahead to offset position
    if (fpos > offset) {
      // rewind
      rc = pf_open((const char *)filename);
      if (rc)
        return;  // quite unlikely
      fpos = 0;
    }
    uint16_t bytes_to_skip = offset - fpos;
    
    // skip to program entry, which is always within first 8K
    while (bytes_to_skip > 0) {
      uint16_t len = bytes_to_skip < sizeof(buffer) ? bytes_to_skip : sizeof(buffer);
      pf_read(buffer, len, &bytes_read);
      if (bytes_read < len)
        return entry_added;  // corrupt image
      bytes_to_skip -= len;
    }
#endif
    
    // analyze metadata
    pf_read(buffer, 6 + ENTRY_NAME_LEN, &bytes_read);
    //fpos = offset + 6 + ENTRY_NAME_LEN;

    uint8_t *p = buffer;
    uint8_t *q = buf_entry;

    // save next entry
    next = (*p++ << 8);
    next += *p++;

    // filename w/o extension
    const uint8_t *r = (const uint8_t *)filename;
    for (i = 8; i > 0 && *r != '.'; --i)
      *q++ = *r++;
    while (i-- > 0)
      *q++ = 0;
    
    // GPL header version
    *q++ = 0;  // also doubles as potential \0 for filename
    *q++ = version;

    // start address in image
    *q++ = *p++;
    *q++ = *p++;

    // next menu entry (ignored for browser entries)
    uint16_t next_entry = 0x6000 + MENU_START +
                          MENU_ENTRY_SIZE * (images_written + 1) + MENU_ENTRY_OFFSET;
    *q++ = next_entry >> 8;
    *q++ = next_entry & 0xff;

    // menu handler (ignored for browser entries)
    *q++ = 0x60;
    *q++ = 0x10 + MENU_HANDLER_SIZE * images_written;
    
    // length and name
    uint8_t len = *p++;
    *q++ = ENTRY_NAME_LEN - 1;
    for (i = 0; i < ENTRY_NAME_LEN; ++i)  // always pad name
      *q++ = i < len ? *p++ : ' ';

    writeMenuEntry();
    writeBrowserEntry();
    entry_added = 1;
    ++images_written;
  }

  return entry_added;
}


/*
 * write entry for browser into RAM
 */

static void writeBrowserEntry()
{
  uint8_t i;
  const uint8_t *p = buf_entry;

  ram_addr = BROWSER_START + BROWSER_ENTRY_SIZE * images_written;

  for (i = MENU_ENTRY_OFFSET; i > 0; --i)
    writeMem(*p++);
  p += 5;  // skip next, start, len
  for (i = ENTRY_NAME_LEN; i > 0; --i)
    writeMem(*p++);
}


/*
 * write menu entry into RAM
 */

static void writeMenuEntry()
{
  if (images_written > 8)
    return;
  
  const uint8_t *p;
  uint8_t i;

  // browser instead of menu entries?
  if (images_written == 8) {
    ram_addr = MENU_START;  // reset menu
    p = browser_entry;
    for (i = sizeof(browser_entry); i > 0; --i, ++p)
      writeMem(pgm_read_byte(p));
    return;
  }

  // write prepared entry to RAM
  ram_addr = MENU_START + MENU_ENTRY_SIZE * images_written;
  p = buf_entry;
  for (i = MENU_ENTRY_SIZE; i > 0; --i)
    writeMem(*p++);
}


/*
 * close both entries lists
 */

static void closeEntries()
{
  uint8_t i;

  if (images_written <= 8) {
    // patch last "next item" entry
    ram_addr = images_written ?
                 MENU_START + MENU_ENTRY_SIZE * images_written - ENTRY_NAME_LEN - 4 : 0x0006;
    writeMem(0);
    writeMem(0);
  }
  ram_addr = BROWSER_START + BROWSER_ENTRY_SIZE * images_written;
  for (i = MENU_ENTRY_OFFSET + 2; i > 0; --i)
    writeMem(0);
}


/*
 * write fixed menu code into SRAM
 */

static void writeMenuCode()
{
  const uint8_t *p = menu;
  ram_addr = 0;
  for (uint16_t i = sizeof(menu); i > 0; --i, ++p)
    writeMem(pgm_read_byte(p));
}


/*
 * wait for image selection by user
 */

void selectImage()
{
  receiveData(buf_entry);
}


/*
 * load selected image from SD card
 */

void loadImage()
{
  uint8_t files_read = 0;
  UINT bytes_read;

  // lock RAM for cart
  disableBus();
  ram_addr = 0;

  rc = pf_mount(&sd_fs);
  if (rc)
    flash_error(1);

  // sender data in buf_entry: F I L E N A M E \0
  uint8_t *q = buf_entry;
  while (*++q);
  uint8_t *p = q--;  // q at last name char == bank indicator
  *p++ = '.'; *p++ = 'B'; *p++ = 'I'; *p++ = 'N'; *p++ = 0;
  
  // load BIN files
#ifdef MULTI_FILE
  while (1) {
#else
  while (files_read == 0) {
#endif
    rc = pf_open((char *)buf_entry);
    if (rc)
      break;
    while (1) {
      rc = pf_read((void *)buffer, sizeof(buffer), &bytes_read);
      if (bytes_read == 0)
        break;
      p = buffer;
      while (bytes_read-- > 0)
        writeMem(*p++);
    }
    if (*q - files_read != 'C')
      break;  // single-file
    ++files_read;
    ++(*q);  // load next BIN
  }

  // release cartridge
  enableBus();
}


/*
 * write single byte to SRAM at current address w/auto increment
 */

static void writeMem(uint8_t byte)
{
  setRAM(ram_addr, byte);

  // mirror byte into unused banks
  if (ram_addr < 0x4000)
    setRAM(ram_addr + 0x4000, byte);
  if (ram_addr < 0x2000) {
    setRAM(ram_addr + 0x2000, byte);
    setRAM(ram_addr + 0x6000, byte);
  }

  ++ram_addr;
}
