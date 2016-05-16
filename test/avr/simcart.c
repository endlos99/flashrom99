/*
 * FlashROM 99 simulator
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "mocktypes.h"
#include <loader.h>
#include <wire.h>


char m_dirname[256];
char m_send[10];


/*
 * mocked hardware
 */

uint8_t ram[32768];

void dumpMemory(const char *fn)
{
  FILE *f = fopen(fn, "wb");
  fwrite(ram, sizeof(uint8_t), sizeof(ram), f);
  fclose(f);
}


/*
 * mocked avr library
 */

uint8_t pgm_read_byte(const uint8_t *p)
{
  return *p;
}


/*
 * mocked wire.c funktions
 */

void disableBus()
{
  for (uint16_t i = 0; i < sizeof(ram); ++i)
    ram[i] = 0xaa;
}

void enableBus()
{
}

void setRAM(uint16_t addr, uint8_t byte)
{
  if (addr > sizeof(ram)) {
    printf("invalid RAM access: %x\n", addr);
    exit(1);
  }
  ram[addr] = byte;
}

void receiveData(uint8_t *buffer)
{
  memcpy((char*)buffer, m_send, 10);
}

void flash_error(const uint8_t count)
{
  printf("flash error: %d", count);
  exit(1);
}


/*
 * mocked pff library
 */

#define AM_DIR	0x10	/* Directory */

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;
typedef unsigned int	UINT;

typedef struct {
  	DWORD	fsize;		/* File size */
	DWORD   clust;
	BYTE	fattrib;	/* Attribute */
	char	fname[13];	/* File name */
} FILINFO;

typedef int FRESULT;


DIR *m_dir = 0;
FILE *m_file = 0;
struct stat m_stat;

FRESULT pf_mount(void* fs)
{
  return 0;
}

FRESULT pf_open(const char* path)
{
  char buf[256];
  strcpy(buf, m_dirname);
  strcat(buf, path);
  printf("open: %s\n", buf);
  m_file = fopen(buf, "rb");
  return !m_file;
}

FRESULT pf_fnoopen(const FILINFO* fno)
{
  char buf[256];
  strcpy(buf, m_dirname);
  strcat(buf, fno->fname);
  printf("fnoopen: %s\n", buf);
  m_file = fopen(buf, "rb");
  return !m_file;
}

FRESULT pf_read(void* buff, UINT btr, UINT* br)
{
  printf("read: %d\n", btr);
  size_t n = fread(buff, 1, btr, m_file);
  *br = n;
  return 0;
}

FRESULT pf_lseek (DWORD ofs)
{
  printf("lseek: %ld\n", ofs);
  fseek(m_file, ofs, SEEK_SET);
  return 0;
}

FRESULT pf_opendir(DIR* dj, const char* path)
{
  printf("opendir: %s\n", m_dirname);
  m_dir = opendir(m_dirname);
  if (!m_dir) {
    printf("error opening %s\n", m_dirname);
    return 1;
  }
  return 0;
}

FRESULT pf_readdir(DIR* dj, FILINFO* fno)
{
  struct dirent *e;
  char buf[256];
  
  while (1) {
    e = readdir(m_dir);
    if (!e) {
      printf("end of dir\n");
      fno->fname[0] = 0;
      return 0;
    }
    if (e->d_name[0] != '.')
      break;
  }

  strcpy(buf, m_dirname);
  strcat(buf, e->d_name);
  stat(buf, &m_stat);
  
  strcpy(fno->fname, e->d_name);
  fno->fsize = m_stat.st_size;
  fno->fattrib = (m_stat.st_mode & S_IFDIR) ? AM_DIR : 0;
  printf("found file: %s (%ld)\n", fno->fname, fno->fsize);
  return 0;
}


/*
 * main program
 */

int main(int argc, char **argv)
{
  // setup test
  if (argc < 4) {
    printf("usage: %s <dir/> <name> <banks>\n", argv[0]);
    exit(1);
  }
  strcpy(m_dirname, argv[1]);
  strcpy(m_send, argv[2]);
  m_send[9] = argv[3][0] - '0';

  // setup cart
  init();

  // create menu loader
  uint8_t dl = genMenu();

  printf("dumping menu memory ...\n");
  dumpMemory("menu.dump");

  // wait for menu selection
  if (!dl)
    selectImage();

  // load selected image
  loadImage();

  printf("dumping loaded image memory ...\n");
  dumpMemory("image.dump");
}
