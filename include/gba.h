#ifndef GBA_H
#define GBA_H
#include <memory.h>
#include <stdint.h>
typedef struct homo_gba {
  memory_t memory;
} gba;

extern gba _gba;

int init_gba(const char* path);
void dele_gba();
#endif
