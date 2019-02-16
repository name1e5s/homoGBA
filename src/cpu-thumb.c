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
#include <cpu-isa.h>
#include <cpu.h>
#include <log.h>
#include <memory.h>
#include <stdbool.h>

#define DSIMM_PA                     \
  uint16_t Rd = opcode & 0x7;        \
  uint16_t Rs = (opcode >> 3) & 0x7; \
  uint16_t imm = (opcode >> 6) & 0x1F;

#define SET_F(N, Z, C, V) \
  cpu.CPSR.N = (N);       \
  cpu.CPSR.Z = (Z);       \
  cpu.CPSR.C = (C);       \
  cpu.CPSR.V = (V);

#define SET_NZC(N, Z, C) SET_F(N, Z, C, F_V)

#define SET_NZ(N, Z) SET_NZC(N, Z, F_C)

#define DECL(BLOCK) static void thumb_##BLOCK(uint16_t opcode)
typedef void (*thumb_block)(uint16_t opcode);

DECL(move_shifted_register) {
  // TODO:
}

DECL(add_sub) {
  // TODO:
}

DECL(move_compare_imm) {
  // TODO:
}

DECL(alu) {
  // TODO:
}

DECL(hi_reg_bx) {
  // TODO:
}

DECL(load_pc_rel) {
  // TODO:
}

DECL(load_store_reg) {
  // TODO:
}

DECL(load_store_se_half) {
  // TODO:
}

DECL(load_store_imm) {
  // TODO:
}

DECL(load_store_half) {
  // TODO:
}

DECL(load_store_sp_rel) {
  // TODO:
}

DECL(get_rel_addr) {
  // TODO:
}

DECL(add_offset_sp) {
  // TODO:
}

DECL(push_pop) {
  // TODO:
}

DECL(multiple_load_store) {
  // TODO:
}

DECL(cond_branch) {
  // TODO:
}

DECL(uncond_branch) {
  // TODO:
}

DECL(long_branch) {
  // TODO:
}

DECL(swi) {
  // TODO:
}

DECL(ill) {
  log_warn("Illegal opcode: 0x%8x", opcode);
  clocks -= 100;  // Away from trap
}

thumb_block thumb_code[] = {
    thumb_ill,
    thumb_move_shifted_register,
    thumb_add_sub,
    thumb_move_compare_imm,
    thumb_alu,
    thumb_hi_reg_bx,
    thumb_load_pc_rel,
    thumb_load_store_reg,
    thumb_load_store_se_half,
    thumb_load_store_imm,
    thumb_load_store_half,
    thumb_load_store_sp_rel,
    thumb_get_rel_addr,
    thumb_add_offset_sp,
    thumb_push_pop,
    thumb_multiple_load_store,
    thumb_cond_branch,
    thumb_swi,
    thumb_uncond_branch,
    thumb_long_branch,
};

uint16_t decode(uint16_t opcode) {
  switch ((opcode >> 13) & 0x7) {
    case 0:
      if (((opcode >> 11) & 0x3) == 3)
        return 2;
      return 1;
    case 1:
      return 3;
    case 2:
      switch ((opcode >> 10) & 7) {
        case 0:
          return 4;
        case 1:
          return 5;
        case 2:
        case 3:
          return 6;
        default:
          if (BIT(opcode, 9))
            return 8;
          return 7;
      }
    case 3:
      return 9;
    case 4:
      if (BIT(opcode, 12))
        return 11;
      return 10;
    case 5:
      if (BIT(opcode, 12) == 0)
        return 12;
      switch ((opcode >> 9) & 0x3) {
        case 0:
          return 13;
        case 2:
          return 14;
        case 3:
          return 17;
        default:
          return 0;
      }
    case 6:
      if (BIT(opcode, 12) == 0)
        return 15;
      switch ((opcode >> 8) & 0xF) {
        case 0xF:
          return 17;
        case 0xE:
          return 0;
        default:
          return 16;
      }
    case 7:
      if (BIT(opcode, 12))
        return 19;
      if (BIT(opcode, 11) == 0)
        return 18;
      break;
    default:
      break;
  }
  return 0;
}

void cpu_run_thumb(int64_t clock) {
  clocks = clock;
  while (clocks > 0) {
    bool isSeq = (cpu.PC_old + 2 == cpu.R[R_PC]);
    cpu.PC_old = cpu.R[R_PC];
    register uint16_t opcode = memory_read_16(cpu.R[R_PC]);
  }
}