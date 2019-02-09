#include <cartridge.h>
#include <gba.h>
#include <stdlib.h>

gba _gba;

int init_gba(const char* path) {
  uint8_t* gamepak = load_cartridge(path);
  _gba.memory.rom_wait0 = gamepak;
  _gba.memory.rom_wait1 = gamepak;
  _gba.memory.rom_wait2 = gamepak;
}

void dele_gba() {
  free(_gba.memory.rom_wait0);
}