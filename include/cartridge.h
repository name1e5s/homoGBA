#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <stdint.h>

typedef struct homo_cartridge_header {
  uint32_t entry_point;
  uint8_t logo[156];
  char title[12];
  char game_code[4];
  char maker_code[2];
  uint8_t fixed_value;
  uint8_t unit;
  uint8_t device;
  uint8_t reversed[7];
  uint8_t version;
  uint8_t checksum;
  uint8_t zero[2];
  // For multiboot
  uint32_t ram_entry_point;
  uint8_t boot_mode;
  uint8_t slave_number;
  uint8_t unused[26];
  uint32_t joybus_entry;
} cartridge_header;

uint8_t* load_cartridge(const char* path);
#endif
