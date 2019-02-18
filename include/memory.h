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

void memory_init(uint8_t* bios_ptr, uint8_t* gamepak_ptr);
void memory_destory(void);

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
// For the macros
#define get_access_cycles_nonseq8(address) get_access_cycles_nonseq16(address)

#define DECL_MEM(bit)                                \
  uint##bit##_t memory_read_##bit(uint32_t address); \
  void memory_write_##bit(uint32_t address, uint##bit##_t value);

DECL_MEM(32)
DECL_MEM(16)
DECL_MEM(8)

    int32_t dma_update(void);
int32_t dma_working(void);
int32_t dma_get_clocks(void);

void dma_request_data(int32_t a, int32_t b);

#define DECLREG(bit)                                  \
  uint##bit##_t register_read_##bit(uint32_t address); \
  void register_write_##bit(uint32_t address, uint##bit##_t value);

DECLREG(32) DECLREG(16) DECLREG(8)

#define REG_(ADDR, TYPE) *((uint##TYPE##_t*)(&memory.io_reg[ADDR - 0x4000000]))

// I/O Registers
// Display Registers
#define DISPCNT 0x4000000
#define GREENSWAP 0x4000002
#define DISPSTAT 0x4000004
#define VCOUNT 0x4000006
#define BG0CNT 0x4000008
#define BG1CNT 0x400000A
#define BG2CNT 0x400000C
#define BG3CNT 0x400000E

#define BG0HOFS 0x4000010
#define BG0VOFS 0x4000012
#define BG1HOFS 0x4000014
#define BG1VOFS 0x4000016
#define BG2HOFS 0x4000018
#define BG2VOFS 0x400001A
#define BG3HOFS 0x400001C
#define BG3VOFS 0x400001E

#define BG2PA 0x4000020
#define BG2PB 0x4000022
#define BG2PC 0x4000024
#define BG2PD 0x4000026

#define BG2X_L 0x4000028
#define BG2X_H 0x400002A
#define BG2Y_L 0x400002C
#define BG2Y_H 0x400002E

#define BG3PA 0x4000030
#define BG3PB 0x4000032
#define BG3PC 0x4000034
#define BG3PD 0x4000036

#define BG3X_L 0x4000038
#define BG3X_H 0x400003A
#define BG3Y_L 0x400003C
#define BG3Y_H 0x400003E

#define WIN0H 0x4000040
#define WIN1H 0x4000042

#define WIN0V 0x4000044
#define WIN1V 0x4000046

#define WININ 0x4000048
#define WINOUT 0x400004A

#define MOSAIC 0x400004C

#define BLDCNT 0x4000050
#define BLDALPHA 0x4000052
#define BLDY 0x4000054

// Sound
#define SND1CNT_L 0x4000060
#define SND1CNT_H 0x4000062
#define SND1CNT_X 0x4000064

#define SND2CNT_L 0x4000068
#define SND2CNT_H 0x400006C

#define SND3CNT_L 0x4000070
#define SND3CNT_H 0x4000072
#define SND3CNT_X 0x4000074

#define SND4CNT_L 0x4000078
#define SND4CNT_H 0x400007C

#define SNDCNT_L 0x4000080
#define SNDCNT_H 0x4000082
#define SNDCNT_X 0x4000084

#define SNDBIAS 0x4000088

#define WAVERAM0_L 0x4000090
#define WAVERAM0_H 0x4000092
#define WAVERAM1_L 0x4000094
#define WAVERAM1_H 0x4000096
#define WAVERAM2_L 0x4000098
#define WAVERAM2_H 0x400009A
#define WAVERAM3_L 0x400009C
#define WAVERAM3_H 0x400009E

#define FIFO_A 0x40000A0
#define FIFO_B 0x40000A4

// Timers
#define TM0CNT_L 0x4000100
#define TM1CNT_L 0x4000104
#define TM2CNT_L 0x4000108
#define TM3CNT_L 0x400010C

#define TM0CNT_H 0x4000102
#define TM1CNT_H 0x4000106
#define TM2CNT_H 0x400010A
#define TM3CNT_H 0x400010E

// Input
#define KEYINPUT 0x4000130
#define KEYCNT 0x4000132

// Interrupts
#define REG_IE 0x4000200
#define REG_IF 0x4000202
#define REG_IME 0x4000208

// Wait Control
#define WAITCNT 0x4000204

// DMA
#define DMA0SAD 0x40000B0
#define DMA1SAD 0x40000BC
#define DMA2SAD 0x40000C8
#define DMA3SAD 0x40000D4

#define DMA0DAD 0x40000B4
#define DMA1DAD 0x40000C0
#define DMA2DAD 0x40000CC
#define DMA3DAD 0x40000D8

#define DMA0CNT_L 0x40000B8
#define DMA1CNT_L 0x40000C4
#define DMA2CNT_L 0x40000D0
#define DMA3CNT_L 0x40000DC

#define DMA0CNT_H 0x40000BA
#define DMA1CNT_H 0x40000C6
#define DMA2CNT_H 0x40000D2
#define DMA3CNT_H 0x40000DE

// GPIO
#define GPIO_DATA 0x80000C4
#define GPIO_DIRECTION 0x80000C6
#define GPIO_CNT 0x80000C8

// Serial IO
#define SIO_DATA_32_L 0x4000120
#define SIO_DATA_32_H 0x4000122
#define SIO_CNT 0x4000128
#define R_CNT 0x4000134
#define SIO_DATA_8 0x400012A

// Display Registers
#define REG_DISPCNT REG_(0x4000000, 16)
#define REG_GREENSWAP REG_(0x4000002, 16)
#define REG_DISPSTAT REG_(0x4000004, 16)
#define REG_VCOUNT REG_(0x4000006, 16)
#define REG_BG0CNT REG_(0x4000008, 16)
#define REG_BG1CNT REG_(0x400000A, 16)
#define REG_BG2CNT REG_(0x400000C, 16)
#define REG_BG3CNT REG_(0x400000E, 16)

#define REG_BG0HOFS REG_(0x4000010, 16)
#define REG_BG0VOFS REG_(0x4000012, 16)
#define REG_BG1HOFS REG_(0x4000014, 16)
#define REG_BG1VOFS REG_(0x4000016, 16)
#define REG_BG2HOFS REG_(0x4000018, 16)
#define REG_BG2VOFS REG_(0x400001A, 16)
#define REG_BG3HOFS REG_(0x400001C, 16)
#define REG_BG3VOFS REG_(0x400001E, 16)

#define REG_BG2PA REG_(0x4000020, 16)
#define REG_BG2PB REG_(0x4000022, 16)
#define REG_BG2PC REG_(0x4000024, 16)
#define REG_BG2PD REG_(0x4000026, 16)

#define REG_BG2X_L REG_(0x4000028, 16)
#define REG_BG2X_H REG_(0x400002A, 16)
#define REG_BG2Y_L REG_(0x400002C, 16)
#define REG_BG2Y_H REG_(0x400002E, 16)

#define REG_BG3PA REG_(0x4000030, 16)
#define REG_BG3PB REG_(0x4000032, 16)
#define REG_BG3PC REG_(0x4000034, 16)
#define REG_BG3PD REG_(0x4000036, 16)

#define REG_BG3X_L REG_(0x4000038, 16)
#define REG_BG3X_H REG_(0x400003A, 16)
#define REG_BG3Y_L REG_(0x400003C, 16)
#define REG_BG3Y_H REG_(0x400003E, 16)

#define REG_WIN0H REG_(0x4000040, 16)
#define REG_WIN1H REG_(0x4000042, 16)

#define REG_WIN0V REG_(0x4000044, 16)
#define REG_WIN1V REG_(0x4000046, 16)

#define REG_WININ REG_(0x4000048, 16)
#define REG_WINOUT REG_(0x400004A, 16)

#define REG_MOSIAC REG_(0x400004C, 16)

#define REG_BLDCNT REG_(0x4000050, 16)
#define REG_BLDALPHA REG_(0x4000052, 16)
#define REG_BLDY REG_(0x4000054, 16)

// Sound
#define REG_SND1CNT_L REG_(0x4000060, 16)
#define REG_SND1CNT_H REG_(0x4000062, 16)
#define REG_SND1CNT_X REG_(0x4000064, 16)

#define REG_SND2CNT_L REG_(0x4000068, 16)
#define REG_SND2CNT_H REG_(0x400006C, 16)

#define REG_SND3CNT_L REG_(0x4000070, 16)
#define REG_SND3CNT_H REG_(0x4000072, 16)
#define REG_SND3CNT_X REG_(0x4000074, 16)

#define REG_SND4CNT_L REG_(0x4000078, 16)
#define REG_SND4CNT_H REG_(0x400007C, 16)

#define REG_SNDCNT_L REG_(0x4000080, 16)
#define REG_SNDCNT_H REG_(0x4000082, 16)
#define REG_SNDCNT_X REG_(0x4000084, 16)

#define REG_SNDBIAS REG_(0x4000088, 16)

#define REG_WAVERAM0_L REG_(0x4000090, 16)
#define REG_WAVERAM0_H REG_(0x4000092, 16)
#define REG_WAVERAM1_L REG_(0x4000094, 16)
#define REG_WAVERAM1_H REG_(0x4000096, 16)
#define REG_WAVERAM2_L REG_(0x4000098, 16)
#define REG_WAVERAM2_H REG_(0x400009A, 16)
#define REG_WAVERAM3_L REG_(0x400009C, 16)
#define REG_WAVERAM3_H REG_(0x400009E, 16)

#define REG_FIFO_A REG_(0x40000A0, 32)
#define REG_FIFO_B REG_(0x40000A4, 32)

// Timers
#define REG_TM0CNT_L REG_(0x4000100, 16)
#define REG_TM1CNT_L REG_(0x4000104, 16)
#define REG_TM2CNT_L REG_(0x4000108, 16)
#define REG_TM3CNT_L REG_(0x400010C, 16)

#define REG_TM0CNT_H REG_(0x4000102, 16)
#define REG_TM1CNT_H REG_(0x4000106, 16)
#define REG_TM2CNT_H REG_(0x400010A, 16)
#define REG_TM3CNT_H REG_(0x400010E, 16)

// Input
#define REG_KEYINPUT REG_(0x4000130, 16)
#define REG_KEYCNT REG_(0x4000132, 16)

// Interrupts
#define REG_REG_IE REG_(0x4000200, 16)
#define REG_REG_IF REG_(0x4000202, 16)
#define REG_REG_IME REG_(0x4000208, 16)

// Wait Control
#define REG_WAITCNT REG_(0x4000204, 16)

// DMA
#define REG_DMA0SAD REG_(0x40000B0, 32)
#define REG_DMA1SAD REG_(0x40000BC, 32)
#define REG_DMA2SAD REG_(0x40000C8, 32)
#define REG_DMA3SAD REG_(0x40000D4, 32)

#define REG_DMA0DAD REG_(0x40000B4, 32)
#define REG_DMA1DAD REG_(0x40000C0, 32)
#define REG_DMA2DAD REG_(0x40000CC, 32)
#define REG_DMA3DAD REG_(0x40000D8, 32)

#define REG_DMA0CNT_L REG_(0x40000B8, 16)
#define REG_DMA1CNT_L REG_(0x40000C4, 16)
#define REG_DMA2CNT_L REG_(0x40000D0, 16)
#define REG_DMA3CNT_L REG_(0x40000DC, 16)

#define REG_DMA0CNT_H REG_(0x40000BA, 16)
#define REG_DMA1CNT_H REG_(0x40000C6, 16)
#define REG_DMA2CNT_H REG_(0x40000D2, 16)
#define REG_DMA3CNT_H REG_(0x40000DE, 16)

// GPIO
#define REG_GPIO_DATA REG_(0x80000C4, 16)
#define REG_GPIO_DIRECTION REG_(0x80000C6, 16)
#define REG_GPIO_CNT REG_(0x80000C8, 16)

// Serial IO
#define REG_SIO_DATA_32_L REG_(0x4000120, 16)
#define REG_SIO_DATA_32_H REG_(0x4000122, 16)
#define REG_SIO_CNT REG_(0x4000128, 16)
#define REG_R_CNT REG_(0x4000134, 16)
#define REG_SIO_DATA_8 REG_(0x400012A, 16)

#endif
