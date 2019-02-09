#include <cartridge.h>
#include <cpu.h>
#include <gba.h>
#include <stdlib.h>

int init_gba(const char* path) {
  uint8_t* gamepak = load_cartridge(path);
  memory.rom_wait0 = gamepak;
  memory.rom_wait1 = gamepak;
  memory.rom_wait2 = gamepak;

  init_cpu();

}

void dele_gba() {
  free(memory.rom_wait0);
}