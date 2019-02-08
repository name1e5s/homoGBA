#ifndef GBA_H
#define GBA_H
#include <stdint.h>
typedef struct homo_gba {
  // External Memory (Game Pak)
  uint8_t* rom_wait0;
  uint8_t* rom_wait1;
  uint8_t* rom_wait2;
} gba;

extern gba _gba;

int init_gba(const char* path);
void dele_gba();
#endif
