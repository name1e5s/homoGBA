#include <gba.h>
#include <memory.h>
#include <stdbool.h>
// Default value. Write to REG_WAITCNT will change them.
uint32_t wait_cycle_seq[16] = {0, 0, 2, 0, 0, 0, 0, 0,
                                      2, 2, 4, 4, 8, 8, 4, 4};
uint32_t wait_cycle_nonseq[16] = {0, 0, 2, 0, 0, 0, 0, 0,
                                         4, 4, 4, 4, 4, 4, 4, 4};

const bool bus_is_16[16] = {0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1};

// Access Cycles
uint32_t get_access_cycles(bool seq, bool quad, uint32_t address) {
  uint32_t idx = (address >> 0x18);
  bool doubled = quad & bus_is_16[idx];
  if (seq)
    return (wait_cycle_seq[idx] + 1) << doubled;
  else if (doubled)
    return (wait_cycle_seq[idx] + wait_cycle_nonseq[idx] + 2);
  return wait_cycle_nonseq[idx] + 1;
}

#define CONVERT_TO(TYPE, DATA) (*(TYPE*)&(DATA))

#define CONVERT_TO_32(DATA) CONVERT_TO(uint32_t, DATA)
#define CONVERT_TO_16(DATA) CONVERT_TO(uint16_t, DATA)
#define CONVERT_TO_8(DATA) CONVERT_TO(uint8_t, DATA)

uint32_t memory_read_32(uint32_t address) {
  uint32_t data;
  if (address < 0x4000)
    data = CONVERT_TO_32(_gba.memory.bios[address]);
  else if (address < 0x03000000)
    data = CONVERT_TO_32(_gba.memory.warm_on_board[address & 0x3FFC]);
  else if (address < 0x04000000)
    data = CONVERT_TO_32(_gba.memory.warm_in_chip[address & 0x7FFC]);
  else if (address < 0x05000000)
    // TODO: Implement The Fucking IO Registers
    return 0;
  else if (address < 0x06000000)
    data = CONVERT_TO_32(_gba.memory.palette[address & 0x3FC]);
  else if (address < 0x06018000)
    return 0;
  else if (address < 0x07000000)
    data = CONVERT_TO_32(_gba.memory.video[address % 0x18000]);
  else if (address < 0x08000000)
    data = CONVERT_TO_32(_gba.memory.obj_attr[address & 0x3FC]);
  else if (address < 0x0e000000)
    data = CONVERT_TO_32(_gba.memory.rom_wait0[address & 0x01FFFFFC]);
  return ((data >> ((address & 0x3) << 3) |
           (data << (32 - ((address & 0x3) << 3)))));
}

void memory_write_32(uint32_t address, uint32_t value) {
  if (address < 0x02000000)
    return;
  if (address < 0x03000000) {
    CONVERT_TO_32(_gba.memory.warm_on_board[address & 0x3FFC]) = value;
    return;
  }
  if (address < 0x04000000) {
    CONVERT_TO_32(_gba.memory.warm_in_chip[address & 0x7FFC]) = value;
    return;
  }
  if (address < 0x05000000) {
    // TODO: Implement the fucking IO Registers
    return;
  }
  if (address < 0x06000000) {
    CONVERT_TO_32(_gba.memory.palette[address & 0x3FC]) = value;
    return;
  }

  if (address < 0x06018000) {
    CONVERT_TO_32(_gba.memory.video[address % 0x18000]) = value;
    return;
  }
  if (address < 0x07000000)
    return;
  if (address < 0x08000000) {
    CONVERT_TO_32(_gba.memory.obj_attr[address & 0x3FC]) = value;
    return;
  }
}

uint16_t memory_read_16(uint32_t address) {
  if (address < 0x4000)
    return CONVERT_TO_16(_gba.memory.bios[address]);
  else if (address < 0x03000000)
    return CONVERT_TO_16(_gba.memory.warm_on_board[address & 0x3FFE]);
  else if (address < 0x04000000)
    return CONVERT_TO_16(_gba.memory.warm_in_chip[address & 0x7FFE]);
  else if (address < 0x05000000)
    // TODO: Implement The Fucking IO Registers
    return 0;
  else if (address < 0x06000000)
    return CONVERT_TO_16(_gba.memory.palette[address & 0x3FE]);
  else if (address < 0x06018000)
    return 0;
  else if (address < 0x07000000)
    return CONVERT_TO_16(_gba.memory.video[address % 0x18000]);
  else if (address < 0x08000000)
    return CONVERT_TO_16(_gba.memory.obj_attr[address & 0x3FE]);
  else if (address < 0x0e000000)
    return CONVERT_TO_16(_gba.memory.rom_wait0[address & 0x01FFFFFE]);
}

void memory_write_16(uint32_t address, uint16_t value) {
  if (address < 0x02000000)
    return;
  if (address < 0x03000000) {
    CONVERT_TO_16(_gba.memory.warm_on_board[address & 0x3FFE]) = value;
    return;
  }
  if (address < 0x04000000) {
    CONVERT_TO_16(_gba.memory.warm_in_chip[address & 0x7FFE]) = value;
    return;
  }
  if (address < 0x05000000) {
    // TODO: Implement the fucking IO Registers
    return;
  }
  if (address < 0x06000000) {
    CONVERT_TO_16(_gba.memory.palette[address & 0x3FFE]) = value;
    return;
  }

  if (address < 0x06018000) {
    CONVERT_TO_16(_gba.memory.video[address % 0x18000]) = value;
    return;
  }
  if (address < 0x07000000)
    return;
  if (address < 0x08000000) {
    CONVERT_TO_16(_gba.memory.obj_attr[address & 0x3FE]) = value;
    return;
  }
}

uint8_t memory_read_8(uint32_t address) {
  if (address < 0x4000)
    return CONVERT_TO_8(_gba.memory.bios[address]);
  else if (address < 0x03000000)
    return CONVERT_TO_8(_gba.memory.warm_on_board[address & 0x3FFF]);
  else if (address < 0x04000000)
    return CONVERT_TO_8(_gba.memory.warm_in_chip[address & 0x7FFF]);
  else if (address < 0x05000000)
    // TODO: Implement The Fucking IO Registers
    return 0;
  else if (address < 0x06000000)
    return CONVERT_TO_8(_gba.memory.palette[address & 0x3FF]);
  else if (address < 0x06018000)
    return 0;
  else if (address < 0x07000000)
    return CONVERT_TO_8(_gba.memory.video[address % 0x18000]);
  else if (address < 0x08000000)
    return CONVERT_TO_8(_gba.memory.obj_attr[address & 0x3FF]);
  else if (address < 0x0e000000)
    return CONVERT_TO_8(_gba.memory.rom_wait0[address & 0x01FFFFFF]);
}

#define EXPAND(val) (((uint16_t)val) | ((uint16_t)val) << 8)
void memory_write_8(uint32_t address, uint8_t value) {
  if (address < 0x02000000)
    return;
  if (address < 0x03000000) {
    CONVERT_TO_8(_gba.memory.warm_on_board[address & 0x3FFE]) = value;
    return;
  }
  if (address < 0x04000000) {
    CONVERT_TO_8(_gba.memory.warm_in_chip[address & 0x7FFE]) = value;
    return;
  }
  if (address < 0x05000000) {
    // TODO: Implement the fucking IO Registers
    return;
  }
  if (address < 0x06000000) {
    CONVERT_TO_16(_gba.memory.palette[address & 0x3FFE]) = EXPAND(value);
    return;
  }

  if (address < 0x06018000) {
    CONVERT_TO_16(_gba.memory.video[address % 0x18000]) = EXPAND(value);
    return;
  }
  if (address < 0x07000000)
    return;
  if (address < 0x08000000) {
    CONVERT_TO_16(_gba.memory.obj_attr[address & 0x3FE]) = EXPAND(value);
    return;
  }
}