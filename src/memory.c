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
#include <apu.h>
#include <gba.h>
#include <int.h>
#include <log.h>
#include <memory.h>
#include <ppu.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>

memory_t memory;

#define SET(TARGET) memset(TARGET, 0, sizeof(TARGET));

// DMA
void dma0_setup(void) {}

void dma1_setup(void) {}
void dma2_setup(void) {}

void dma3_setup(void) {}

uint32_t wait_table_seq[16] = {0, 0, 2, 0, 0, 0, 0, 0,
                               2, 2, 4, 4, 8, 8, 4, 4};  // Defaults
uint32_t wait_table_nonseq[16] = {0, 0, 2, 0, 0, 0, 0, 0,
                                  4, 4, 4, 4, 4, 4, 4, 4};
uint32_t sram_wait[4] = {4, 3, 2, 8};
uint32_t rom_wait_nonseq[4] = {4, 3, 2, 8};
uint32_t rom_wait_seq0[2] = {2, 1};
uint32_t rom_wait_seq1[2] = {4, 1};
uint32_t rom_wait_seq2[2] = {8, 1};

static inline void memory_cycle_update(void) {
  uint32_t data = REG_WAITCNT;

  uint32_t sram_clocks = sram_wait[data & 3];
  wait_table_seq[14] = sram_clocks;
  wait_table_seq[15] = sram_clocks;
  wait_table_nonseq[14] = sram_clocks;
  wait_table_nonseq[15] = sram_clocks;

  uint32_t rom0_nonseq = rom_wait_nonseq[(data >> 2) & 3];
  wait_table_nonseq[8] = rom0_nonseq;
  wait_table_nonseq[9] = rom0_nonseq;

  uint32_t rom0_seq = rom_wait_seq0[(data >> 4) & 1];
  wait_table_seq[8] = rom0_seq;
  wait_table_seq[9] = rom0_seq;

  uint32_t rom1_nonseq = rom_wait_nonseq[(data >> 5) & 3];
  wait_table_nonseq[10] = rom1_nonseq;
  wait_table_nonseq[11] = rom1_nonseq;

  uint32_t rom1_seq = rom_wait_seq1[(data >> 7) & 1];
  wait_table_seq[10] = rom1_seq;
  wait_table_seq[11] = rom1_seq;

  uint32_t rom2_nonseq = rom_wait_nonseq[(data >> 8) & 3];
  wait_table_nonseq[12] = rom2_nonseq;
  wait_table_nonseq[13] = rom2_nonseq;

  uint32_t rom2_seq = rom_wait_seq2[(data >> 10) & 1];
  wait_table_seq[12] = rom2_seq;
  wait_table_seq[13] = rom2_seq;
}

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

  memory_cycle_update();
  dma0_setup();
  dma1_setup();
  dma2_setup();
  dma3_setup();
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
  uint32_t idx = (address >> 0x18) & 0xF;
  bool doubled = quad && bus_is_16[idx];
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
    return register_read_32(address);
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
    register_write_32(address, value);
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
    return register_read_16(address);
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
    register_write_16(address, value);
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
    return register_read_8(address);
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
    memory_write_8(address, value);
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

static const bool register_canread[] = {
    true,  true,  true,  true,  true,  true,  true,  true,  false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, true,  true,  false, false, true,  false, false, false,
    false, false, false, false, true,  true,  true,  false, true,  false, true,
    false, true,  true,  true,  false, true,  false, true,  false, true,  true,
    true,  false, true,  false, false, false, true,  true,  true,  true,  true,
    true,  true,  true,  false, false, false, false, false, false, false, false,
    false, false, false, false, false, true,  false, false, false, false, false,
    true,  false, false, false, false, false, true,  false, false, false, false,
    false, true,  false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, true,  true,  true,  true,
    true,  true,  true,  true,  false, false, false, false, false, false, false,
    false, true,  true,  true,  true,  true,  true,  false, false, true,  true,
    true,  false, false, false, false, false, true,  false, false, false, false,
    false, false, false, true,  true,  true,  true,  true,  false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, true,  true,  true,  false, true,  false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, true,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false};

// The Fucking I/O Registers
uint16_t register_read_16(uint32_t address) {
  if (address > 0x4000400)
    return 0;
  if (register_canread[(address & 0x3FF) >> 1])
    return REG_(address, 16);
  return 0;
}

uint32_t register_read_32(uint32_t address) {
  return (((uint32_t)register_read_16(address + 2) << 16) |
          ((uint32_t)register_read_16(address)));
}

uint8_t register_read_8(uint32_t address) {
  if (address & 0x1)
    return (uint8_t)(register_read_16(address & ~1) >> 8);
  else
    return (uint8_t)(register_read_16(address & ~1) & 0xFF);
}

void register_write_16(uint32_t address, uint16_t value) {
  if (address > 0x4000400)
    return;
  switch ((address & 0x3FF) + 0x4000000) {
    case DISPCNT:
      REG_DISPCNT = value;
      ppu_update_draw_scanline();
      return;
    case DISPSTAT:
      REG_DISPSTAT = (REG_DISPSTAT & 7) | (value & 0xFFF8);
      if ((REG_DISPCNT >> 8) != (value >> 8)) {
        if ((REG_DISPSTAT >> 8) == REG_VCOUNT) {
          REG_DISPSTAT |= ONE(2);
          int_LCD(ONE(5));
        } else
          REG_DISPSTAT &= ~(ONE(2));
      }
      return;
    case VCOUNT:
    case KEYINPUT:
      return;
#define BG(N)                        \
  case BG##N##HOFS:                  \
    REG_BG##N##HOFS = value & 0x1FF; \
    return;                          \
  case BG##N##VOFS:                  \
    REG_BG##N##VOFS = value & 0x1FF; \
    return;

      BG(0)
      BG(1)
      BG(2)
      BG(3)

#define BG_(N)                       \
  case BG##N##X_L:                   \
    REG_BG##N##X_L = value;          \
    ppu_update_register(BG##N##X_L); \
    return;                          \
  case BG##N##X_H:                   \
    REG_BG##N##X_H = value & 0x0FFF; \
    ppu_update_register(BG##N##X_H); \
    return;                          \
  case BG##N##Y_L:                   \
    REG_BG##N##Y_L = value;          \
    ppu_update_register(BG##N##Y_L); \
    return;                          \
  case BG##N##Y_H:                   \
    REG_BG##N##Y_H = value & 0x0FFF; \
    ppu_update_register(BG##N##Y_H); \
    return;

      BG_(2)
      BG_(3)

#define WIN(NU)                   \
  case WIN##NU:                   \
    REG_WIN##NU = value;          \
    ppu_update_register(WIN##NU); \
    return;

      WIN(0H)
      WIN(0V)
      WIN(1H)
      WIN(1V)
      WIN(IN) WIN(OUT)

          case MOSAIC : REG_MOSIAC = value;
      ppu_update_register(MOSAIC);
      return;

#define DMA(NU)                 \
  case DMA##NU##CNT_H:          \
    REG_DMA##NU##CNT_H = value; \
    dma##NU##_setup();          \
    return;

      DMA(0)
      DMA(1)
      DMA(2)
      DMA(3)

#define TM(NU)                   \
  case TM##NU##CNT_L:            \
    timer_set_start_##NU(value); \
    return;                      \
  case TM##NU##CNT_H:            \
    REG_TM##NU##CNT_H = value;   \
    timer_setup_##NU();          \
    return;

      TM(0)
      TM(1)
      TM(2)
      TM(3)

      // D.N.E.
#define SND4CNT_X 0x4000007D
#define REG_SND4CNT_X REG_(0x400007D, 16)

#define SND2CNT_X 0x4000000D
#define REG_SND2CNT_X REG_(0x400007D, 16)
#define SND_(NU)                                 \
  case SND##NU##CNT_L:                           \
    REG_SND##NU##CNT_L = value;                  \
    apu_register_write16(SND##NU##CNT_L, value); \
    return;                                      \
  case SND##NU##CNT_H:                           \
    REG_SND##NU##CNT_H = value;                  \
    apu_register_write16(SND##NU##CNT_H, value); \
    return;                                      \
  case SND##NU##CNT_X:                           \
    REG_SND##NU##CNT_X = value;                  \
    apu_register_write16(SND##NU##CNT_X, value); \
    return;

          SND_(1) SND_(2) SND_(3) SND_(4) case SNDCNT_L : REG_SNDCNT_L = value;
      apu_register_write16(SNDCNT_L, value);
      return;
    case SNDCNT_H:
      REG_SNDCNT_H = value;
      apu_register_write16(SNDCNT_H, value);
      return;
    case SNDCNT_X:
      REG_SNDCNT_X &= 0xF;
      REG_SNDCNT_X |= value & 0xFFF0;
      apu_register_write16(SNDCNT_X, value & 0xFFF0);
      return;
    case SNDBIAS:
      REG_SNDBIAS = value;
      apu_register_write16(SNDBIAS, value);
      return;

#define FIFO(TY, NU)                             \
  case FIFO_##TY + NU:                           \
    REG_(FIFO_##TY + NU, 16) = value;            \
    apu_register_write16(FIFO_##TY + NU, value); \
    return;

      FIFO(A, 0)
      FIFO(A, 2)
      FIFO(B, 0)
      FIFO(B, 2)

    case WAVERAM0_H ... WAVERAM3_L:
      REG_(address, 16) = value;
      apu_register_write16(address, value);
      return;

    case REG_IE:
      REG_REG_IE = value;
      return;
    case REG_IF:
      REG_REG_IF = ~value;
      return;
    case WAITCNT:
      REG_WAITCNT = value;
      memory_cycle_update();
      return;
    case REG_IME:
      REG_REG_IME = value;
      return;
    case 0x4000128:
      return;

    default:
      REG_(address, 16) = value;
      return;
  }
}

void register_write_32(uint32_t address, uint32_t value) {
  register_write_16(address, (uint16_t)value & 0xFFFF);
  register_write_16(address, (uint16_t)(value >> 16));
}

void register_write_8(uint32_t address, uint8_t value) {
  uint16_t tmp = register_read_16(address & ~1);
  if (address & 1)
    register_write_16(address & ~1, (tmp & 0x00FF) | (((uint16_t)value) << 8));
  else
    register_write_16(address & ~1, (tmp & 0xFF00) | (uint16_t)value);
}