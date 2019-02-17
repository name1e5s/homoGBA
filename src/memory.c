/*
 * This file is part of homoGBA.
 * Copyright (C) 2019  name1e5s<name1e5s@qq.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <gba.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>"

memory_t memory;

#define SET(TARGET) memset(TARGET, 0, sizeof(TARGET));
void memory_init(uint8_t* bios_ptr, uint8_t* gamepak_ptr) {
  memory.bios = bios_ptr;
  memory.rom_wait0 = gamepak_ptr;
  memory.rom_wait1 = gamepak_ptr;
  memory.rom_wait2 = gamepak_ptr;

  SET(memory.warm_in_chip);
  SET(memory.warm_on_board);
  SET(memory.palette)
  SET(memory.video)
  SET(memory.obj_attr)
  SET(memory.io_reg)

  // Set registers
  register_write_16(DISPCNT, 0x0080);
  register_write_16(GREENSWAP, 0);
  register_write_16(DISPSTAT, 0);
  register_write_16(VCOUNT, 0);
  register_write_16(BG0CNT, 0);
  register_write_16(BG1CNT, 0);
  register_write_16(BG2CNT, 0);
  register_write_16(BG3CNT, 0);
  register_write_16(BG0HOFS, 0);
  register_write_16(BG0VOFS, 0);
  register_write_16(BG1HOFS, 0);
  register_write_16(BG1VOFS, 0);
  register_write_16(BG2HOFS, 0);
  register_write_16(BG2VOFS, 0);
  register_write_16(BG3HOFS, 0);
  register_write_16(BG3VOFS, 0);
  register_write_16(BG2PA, 1 << 8);
  register_write_16(BG2PB, 0);
  register_write_16(BG2PC, 0);
  register_write_16(BG2PD, 1 << 8);
  register_write_16(BG2X_L, 0);
  register_write_16(BG2X_H, 0);
  register_write_16(BG2Y_L, 0);
  register_write_16(BG2Y_H, 0);
  register_write_16(BG3PA, 1 << 8);
  register_write_16(BG3PB, 0);
  register_write_16(BG3PC, 0);
  register_write_16(BG3PD, 1 << 8);
  register_write_16(BG3X_L, 0);
  register_write_16(BG3X_H, 0);
  register_write_16(BG3Y_L, 0);
  register_write_16(BG3Y_H, 0);
  register_write_16(WIN0H, 0);
  register_write_16(WIN1H, 0);
  register_write_16(WIN0V, 0);
  register_write_16(WIN1V, 0);
  register_write_16(WININ, 0);
  register_write_16(WINOUT, 0);
  register_write_16(MOSAIC, 0);
  register_write_16(BLDCNT, 0);
  register_write_16(BLDALPHA, 0);
  register_write_16(BLDY, 0);

  register_write_16(SND1CNT_L, 0);
  register_write_16(SND1CNT_H, 0);
  register_write_16(SND1CNT_X, 0);
  register_write_16(SND2CNT_L, 0);
  register_write_16(SND2CNT_H, 0);
  register_write_16(SND3CNT_L, 0);
  register_write_16(SND3CNT_H, 0);
  register_write_16(SND3CNT_X, 0);
  register_write_16(SND4CNT_L, 0);
  register_write_16(SND4CNT_H, 0);
  register_write_16(SNDCNT_L, 0);
  register_write_16(SNDCNT_H, 0x880E);
  register_write_16(SNDCNT_X, 0);
  register_write_16(SNDBIAS, 0);
  register_write_32(FIFO_A, 0);
  register_write_32(FIFO_B, 0);

  register_write_32(DMA0SAD, 0);
  register_write_32(DMA0DAD, 0);
  register_write_16(DMA0CNT_L, 0);
  register_write_16(DMA0CNT_H, 0);
  register_write_32(DMA1SAD, 0);
  register_write_32(DMA1DAD, 0);
  register_write_16(DMA1CNT_L, 0);
  register_write_16(DMA1CNT_H, 0);
  register_write_32(DMA2SAD, 0);
  register_write_32(DMA2DAD, 0);
  register_write_16(DMA2CNT_L, 0);
  register_write_16(DMA2CNT_H, 0);
  register_write_32(DMA3SAD, 0);
  register_write_32(DMA3DAD, 0);
  register_write_16(DMA3CNT_L, 0);
  register_write_16(DMA3CNT_H, 0);

  register_write_16(TM0CNT_L, 0xFF6F);
  register_write_16(TM0CNT_H, 0);
  register_write_16(TM1CNT_L, 0);
  register_write_16(TM1CNT_H, 0);
  register_write_16(TM2CNT_L, 0);
  register_write_16(TM2CNT_H, 0);
  register_write_16(TM3CNT_L, 0);
  register_write_16(TM3CNT_H, 0);

  register_write_16(KEYINPUT, 0);
  register_write_16(KEYCNT, 0);

  register_write_16(REG_IE, 0);
  register_write_16(REG_IF, 0);
  register_write_16(WAITCNT, 0);
  register_write_16(REG_IME, 0);
}

void memory_destory(void) {
  free(memory.bios);
  free(memory.rom_wait0);
}

// Default value. Write to REG_WAITCNT will change them.
uint32_t wait_cycle_seq[16] = {0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 4, 4, 8, 8, 4, 4};
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
    data = CONVERT_TO_32(memory.bios[address]);
  else if (address < 0x03000000)
    data = CONVERT_TO_32(memory.warm_on_board[address & 0x3FFC]);
  else if (address < 0x04000000)
    data = CONVERT_TO_32(memory.warm_in_chip[address & 0x7FFC]);
  else if (address < 0x05000000)
    // TODO: Implement The Fucking IO Registers
    return 0;
  else if (address < 0x06000000)
    data = CONVERT_TO_32(memory.palette[address & 0x3FC]);
  else if (address < 0x06018000)
    return 0;
  else if (address < 0x07000000)
    data = CONVERT_TO_32(memory.video[address % 0x18000]);
  else if (address < 0x08000000)
    data = CONVERT_TO_32(memory.obj_attr[address & 0x3FC]);
  else if (address < 0x0e000000)
    data = CONVERT_TO_32(memory.rom_wait0[address & 0x01FFFFFC]);
  return ((data >> ((address & 0x3) << 3) |
           (data << (32 - ((address & 0x3) << 3)))));
}

void memory_write_32(uint32_t address, uint32_t value) {
  if (address < 0x02000000)
    return;
  if (address < 0x03000000) {
    CONVERT_TO_32(memory.warm_on_board[address & 0x3FFC]) = value;
    return;
  }
  if (address < 0x04000000) {
    CONVERT_TO_32(memory.warm_in_chip[address & 0x7FFC]) = value;
    return;
  }
  if (address < 0x05000000) {
    // TODO: Implement the fucking IO Registers
    return;
  }
  if (address < 0x06000000) {
    CONVERT_TO_32(memory.palette[address & 0x3FC]) = value;
    return;
  }

  if (address < 0x06018000) {
    CONVERT_TO_32(memory.video[address % 0x18000]) = value;
    return;
  }
  if (address < 0x07000000)
    return;
  if (address < 0x08000000) {
    CONVERT_TO_32(memory.obj_attr[address & 0x3FC]) = value;
    return;
  }
}

uint16_t memory_read_16(uint32_t address) {
  if (address < 0x4000)
    return CONVERT_TO_16(memory.bios[address]);
  else if (address < 0x03000000)
    return CONVERT_TO_16(memory.warm_on_board[address & 0x3FFE]);
  else if (address < 0x04000000)
    return CONVERT_TO_16(memory.warm_in_chip[address & 0x7FFE]);
  else if (address < 0x05000000)
    // TODO: Implement The Fucking IO Registers
    return 0;
  else if (address < 0x06000000)
    return CONVERT_TO_16(memory.palette[address & 0x3FE]);
  else if (address < 0x06018000)
    return 0;
  else if (address < 0x07000000)
    return CONVERT_TO_16(memory.video[address % 0x18000]);
  else if (address < 0x08000000)
    return CONVERT_TO_16(memory.obj_attr[address & 0x3FE]);
  else if (address < 0x0e000000)
    return CONVERT_TO_16(memory.rom_wait0[address & 0x01FFFFFE]);
}

void memory_write_16(uint32_t address, uint16_t value) {
  if (address < 0x02000000)
    return;
  if (address < 0x03000000) {
    CONVERT_TO_16(memory.warm_on_board[address & 0x3FFE]) = value;
    return;
  }
  if (address < 0x04000000) {
    CONVERT_TO_16(memory.warm_in_chip[address & 0x7FFE]) = value;
    return;
  }
  if (address < 0x05000000) {
    // TODO: Implement the fucking IO Registers
    return;
  }
  if (address < 0x06000000) {
    CONVERT_TO_16(memory.palette[address & 0x3FFE]) = value;
    return;
  }

  if (address < 0x06018000) {
    CONVERT_TO_16(memory.video[address % 0x18000]) = value;
    return;
  }
  if (address < 0x07000000)
    return;
  if (address < 0x08000000) {
    CONVERT_TO_16(memory.obj_attr[address & 0x3FE]) = value;
    return;
  }
}

uint8_t memory_read_8(uint32_t address) {
  if (address < 0x4000)
    return CONVERT_TO_8(memory.bios[address]);
  else if (address < 0x03000000)
    return CONVERT_TO_8(memory.warm_on_board[address & 0x3FFF]);
  else if (address < 0x04000000)
    return CONVERT_TO_8(memory.warm_in_chip[address & 0x7FFF]);
  else if (address < 0x05000000)
    // TODO: Implement The Fucking IO Registers
    return 0;
  else if (address < 0x06000000)
    return CONVERT_TO_8(memory.palette[address & 0x3FF]);
  else if (address < 0x06018000)
    return 0;
  else if (address < 0x07000000)
    return CONVERT_TO_8(memory.video[address % 0x18000]);
  else if (address < 0x08000000)
    return CONVERT_TO_8(memory.obj_attr[address & 0x3FF]);
  else if (address < 0x0e000000)
    return CONVERT_TO_8(memory.rom_wait0[address & 0x01FFFFFF]);
}

#define EXPAND(val) (((uint16_t)val) | ((uint16_t)val) << 8)
void memory_write_8(uint32_t address, uint8_t value) {
  if (address < 0x02000000)
    return;
  if (address < 0x03000000) {
    CONVERT_TO_8(memory.warm_on_board[address & 0x3FFE]) = value;
    return;
  }
  if (address < 0x04000000) {
    CONVERT_TO_8(memory.warm_in_chip[address & 0x7FFE]) = value;
    return;
  }
  if (address < 0x05000000) {
    // TODO: Implement the fucking IO Registers
    return;
  }
  if (address < 0x06000000) {
    CONVERT_TO_16(memory.palette[address & 0x3FFE]) = EXPAND(value);
    return;
  }

  if (address < 0x06018000) {
    CONVERT_TO_16(memory.video[address % 0x18000]) = EXPAND(value);
    return;
  }
  if (address < 0x07000000)
    return;
  if (address < 0x08000000) {
    CONVERT_TO_16(memory.obj_attr[address & 0x3FE]) = EXPAND(value);
    return;
  }
}
