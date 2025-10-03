// This file uses the 8080 emulator to run the test suite (roms in cpu_tests
// directory). It uses a simple array as memory.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"

#include "drivers/picocalc.h"
#include "drivers/keyboard.h"
#include "drivers/onboard_led.h"
#include "drivers/fat32.h"

#include "i8080.h"
#include <unistd.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>

volatile bool keyIsReady = false;

void keyReadyCallback()
{
  keyIsReady = true;
}

// memory callbacks
#define MEMORY_SIZE 0x10000
static uint8_t* memory = NULL;

static uint8_t rb(void* userdata, uint16_t addr) {
  return memory[addr];
}

static void wb(void* userdata, uint16_t addr, uint8_t val) {
  // We just return so as to simulate rom at the highest addr
  if (addr == 0xffff)
    return;
  memory[addr] = val;
}

static char lastAlphaTyped;
bool tapeInProgress = false;
static fat32_file_t tapeF;
static int tapeNullCountdown = 3;
static bool sourceInProgress = false;
static fat32_file_t sFile;

static uint8_t port_in(void* userdata, uint8_t port) {
  if (port == 0x01 || port == 0x11)
  {

    // but is sourcing in progress?
    if (sourceInProgress)
    {
      size_t bytes_read;
      char sChr;
      fat32_read(&sFile, &sChr, 1, &bytes_read);
      if (bytes_read != 1)
      {
        sChr = 0x0d;
        sourceInProgress = false;
        fat32_close(&sFile);
        printf("\nDone sourcing\n");
      }
      keyIsReady = false;
      return sChr;
    }

    if (!keyIsReady)
    {
      //printf("read while not ready port 1\n");
      fflush(stdout);
      return 0x00;
    }
    
    // it wants character input
    keyIsReady = false;
    char chr = getchar();
    if (chr == 0x0a)
    {
      return 0x0d;
    }
    else if (chr == 0x09)
    {
      // ctrl initiates a redirect from a text file
      // to source in to basic
      printf("Enter file name to source: /Altair/");
      char path[50]="/Altair/";
      char *nextChr = &path[7];
      while ((*(++nextChr) = getchar()) != 0x0d)
      {
        printf("%c", *nextChr);
        if (*nextChr == '_')
          nextChr = nextChr - 2;
      }
      *nextChr = 0;

      printf("Opening %s\n",path);
      fat32_error_t status =fat32_open(&sFile, path);
      if (status != FAT32_OK)
        fprintf(stderr, "Error: %s\n", fat32_error_string(status));
      sourceInProgress = true;
      size_t bytes_read;
      fat32_read(&sFile, &chr, 1, &bytes_read);
      return chr;
    }


    // invert the sense of the shift key for alphas 
    // basic keywords are all caps...
    if (chr >= 'A' && chr <= 'Z')
    {
      lastAlphaTyped = chr; //save the upper case version
      chr = tolower(chr);
    }
    else if (chr >= 'a' && chr <= 'z')
    {
      chr = toupper(chr);
      lastAlphaTyped = chr; // save the upper case version
    }
    return chr;
  }
  else if (port == 0)
  {
    if (keyIsReady||sourceInProgress)
    {
      return 0x00;
    }
    else
    {
      return 0x01;
    }
  }
  else if (port == 0x10)
  {
    if (keyIsReady||sourceInProgress)
    {
      return 0x7f;
    }
    else
    {
      return 0xff;
    }

  }
  else if (port == 6)
  {
    return 0x00; // tape always ready to read/write a char
  }
  else if (port == 7)
  {
    if (!tapeInProgress)
    {
      // open a tape file
      char path[25];
      sprintf(path, "/Altair/tapes/tape_%c.dat", lastAlphaTyped);
      fat32_error_t status = 
        fat32_open(&tapeF, path);
      if (status != FAT32_OK)
      {
        fprintf(stderr, "Error: %d, There is a problem opening %s for read\n",status,path);
      }
      else
        tapeInProgress = true;     
    }
    char tchar;
    size_t bytes_read;
    if (fat32_read(&tapeF, &tchar, 1, &bytes_read) != FAT32_OK)
    {
      fprintf(stderr, "Error reading bytes from tape file\n");
      return 0;
    }

    if (bytes_read == 1)
    {
      return tchar;
    }
    else 
    {
      tapeInProgress = false;
      return 0;
    }
  }

  // uncomment the following if you are curious about
  // other ports that are accessed
  printf("IN from port %02x\n",port);
  return 0x00;
}

static char last0=0xff, last1=0xff, last2=0xff;

static void port_out(void* userdata, uint8_t port, uint8_t value) {
  i8080* const c = (i8080*) userdata;

  if (port == 0x18 || port == 0x01 || port == 0x11)
  {
    putchar(c->a & 0x7f);
    return;
  }
  else if (port == 7)
  {
    if (!tapeInProgress)
    {
      // open a tape file
      char path[25];
      sprintf(path, "/Altair/tapes/tape_%c.dat", lastAlphaTyped);
      fat32_delete(path);// in case it's been written before
      fat32_error_t status = fat32_create(&tapeF, path);
      if (status != FAT32_OK)
      {
        fprintf(stderr, "Error: %d There is a problem opening %s for write\n",status, path);
      }
      else
      {
        tapeInProgress = true;
        tapeNullCountdown = 3;
      }     
    }

    // just write the char  to the file
    size_t bytes_written;
    if (fat32_write(&tapeF, &c->a, 1, &bytes_written) != FAT32_OK)
    {
      fprintf(stderr, "Error writing byte to tape file.\n");
    }
    else if (bytes_written != 1)
    {
      fprintf(stderr, "Error writing byte to tape, bytes read = %d rather than the expected 1\n", bytes_written);
    }

    //look for the pattern: 0x00,0x00,0x00 that indicates
    //the end of writing.
    last0 = last1;
    last1 = last2;
    last2 = c->a;
    if (last0==0 && last1==0 && last2==0)
    {
      fat32_close(&tapeF);
      printf("Wrote tape file.\n");
      tapeInProgress = false;
    }
    
  }

  // uncomment the following for info about 
  // outs to ports
  printf("Out to port: %02x = %02x\n", port, value);
}

static inline int load_file(const char* filename, uint16_t addr) {
  printf("Loading %s\n", filename);
  fat32_file_t f;
  if (fat32_open(&f, filename) != FAT32_OK){
    fprintf(stderr, "error: can't open file '%s'.\n", filename);
    return 1;
  }
  else
  {
    fprintf(stderr, "File opened\n");
  }

  // file size check:
  uint32_t file_size = fat32_size(&f);
  printf("File size is %ld bytes.\n", file_size);

  if (file_size + addr >= MEMORY_SIZE) {
    fprintf(stderr, "error: file %s can't fit in memory.\n", filename);
    return 1;
  }
  else
  {
    fprintf(stderr, "file fits in memory\n");
  }

  // copying the bytes in memory:
  size_t bytes_read;
  fat32_error_t status= fat32_read(&f, &memory[addr],
                         file_size, &bytes_read);

  if (status != FAT32_OK) {
    fprintf(stderr, "error: while reading file '%s'\n", filename);
    return 1;
  }
  else
  {
    fprintf(stderr, "File read in ok\n");
  }

  fat32_close(&f);
  return 0;
}

static inline void run_test(
    i8080* const c, const char* filename, unsigned long cyc_expected) {
  i8080_init(c);
  c->userdata = c;
  c->read_byte = rb;
  c->write_byte = wb;
  c->port_in = port_in;
  c->port_out = port_out;
  memset(memory, 0, MEMORY_SIZE);

  // simulate one byte of unwritable memory at the top end
  // to keep altair basic from killing itself looking for high
  // rom for it's memory sizing
  memory[MEMORY_SIZE-1] = 0xff; // always read as ff

  if (load_file(filename, 0) != 0) {
    return;
  }

  c->pc = 0x00;
  keyIsReady = false;
  while (1) {
    i8080_step(c);
  }

}

bool power_off_requested = false;
volatile bool user_interrupt = false;

int main(void) {

    stdio_init_all();
    picocalc_init(NULL);
    keyboard_init(keyReadyCallback);

  memory = malloc(MEMORY_SIZE);
  if (memory == NULL) {
    return 1;
  }

  i8080 cpu;
  run_test(&cpu, "/Altair/basicload.bin", 0);

  free(memory);

  return 0;
}
