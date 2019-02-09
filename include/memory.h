#ifndef MEMORY_H
#define MEMORY_H
#include <macros.h>
#include <stdbool.h>
#include <stdint.h>
typedef struct homo_memory {
  // General Internal Memory
  uint8_t* bios;  // 00000000-00003FFF   BIOS - System ROM
  uint8_t
      warm_on_board[256 _K];    // 02000000-0203FFFF   WRAM - On-board Work RAM
  uint8_t warm_in_chip[32 _K];  // 03000000-03007FFF   WRAM - On-chip Work RAM
  uint8_t io_reg[0x3FF];        // 04000000-040003FE   I/O Registers

  // Internal Display Memory
  uint8_t palette[1 _K];   // 05000000-050003FF   BG/OBJ Palette RAM
  uint8_t video[96 _K];    // 06000000-06017FFF   VRAM - Video RAM
  uint8_t obj_attr[1 _K];  // 07000000-070003FF   OAM - OBJ Attributes

  // External Memory (Game Pak)
  uint8_t* rom_wait0;
  uint8_t* rom_wait1;
  uint8_t* rom_wait2;
} memory_t;

extern memory_t memory;

uint32_t get_access_cycles(bool seq, bool quad, uint32_t address);
// IN COMPILER WE TRUST
#define get_access_cycles_nonseq(quad, address) \
  get_access_cycles(false, quad, address)
#define get_access_cycles_nonseq32(address) \
  get_access_cycles(false, true, address)
#define get_access_cycles_nonseq16(address) \
  get_access_cycles(false, false, address)
#define get_access_cycles_seq(quad, address) \
  get_access_cycles(true, quad, address)
#define get_access_cycles_seq32(address) get_access_cycles(true, true, address)
#define get_access_cycles_seq16(address) get_access_cycles(true, false, address)

#define DECL_MEM(bit)                                \
  uint##bit##_t memory_read_##bit(uint32_t address); \
  void memory_write_##bit(uint32_t address, uint##bit##_t value);

    DECL_MEM(32) DECL_MEM(16) DECL_MEM(8)

#endif
