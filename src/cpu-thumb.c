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

#define SET_F(SN, SZ, SC, SV) \
  cpu.CPSR.N = (SN);          \
  cpu.CPSR.Z = (SZ);          \
  cpu.CPSR.C = (SC);          \
  cpu.CPSR.V = (SV);

#define SET_NZC(SN, SZ, SC) SET_F(SN, SZ, SC, F_V)

#define SET_NZ(SN, SZ) SET_F(SN, SZ, F_C, F_V)

#define DECL(BLOCK) static void thumb_##BLOCK(uint16_t opcode)
typedef void (*thumb_block)(uint16_t opcode);

bool isSeq;

DECL(move_shifted_register) {
  uint16_t Rd = (uint16_t)(opcode & 0x7);
  uint16_t Rs = (uint16_t)((opcode >> 3) & 0x7);
  uint8_t imm = (uint8_t)((opcode >> 6) & 0x1F);
  switch ((opcode >> 11) & 3) {
    case 0:
      cpu.R[Rd] = lsl_by_imm_carry(cpu.R[Rs], imm);
      break;
    case 1:
      cpu.R[Rd] = lsr_by_imm_carry(cpu.R[Rs], imm);
      break;
    case 2:
      cpu.R[Rd] = asr_by_imm_carry(cpu.R[Rs], imm);
      break;
    default:
      break;
  }
  SET_NZ(BIT(cpu.R[Rd], 31), cpu.R[Rd] == 0)
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

DECL(add_sub) {
  uint16_t Rd = (uint16_t)(opcode & 0x7);
  uint16_t Rs = (uint16_t)((opcode >> 3) & 0x7);
  uint16_t Rn = (uint16_t)((opcode >> 6) & 0x7);

  uint64_t reg = (uint64_t)(uint32_t)(~cpu.R[Rn]) + 1;
  uint64_t imm = (uint64_t)(uint32_t)(~Rn) + 1;
  uint64_t temp = 0;
  switch ((opcode >> 9) & 0x3) {
    case 0:
      temp = (uint64_t)cpu.R[Rs] + (uint64_t)cpu.R[Rn];
      cpu.R[Rd] = (uint32_t)temp;
      SET_F(cpu.R[Rd] >> 31, cpu.R[Rd] == 0, (temp >> 32) != 0,
            V_ADD(cpu.R[Rn], cpu.R[Rn], cpu.R[Rd]))
      break;
    case 1:
      temp = (uint64_t)cpu.R[Rs] + reg;
      cpu.R[Rd] = (uint32_t)temp;
      SET_F(cpu.R[Rd] >> 31, cpu.R[Rd] == 0, (temp >> 32) != 0,
            V_ADD(cpu.R[Rn], (uint32_t)(reg - 1), cpu.R[Rd]))
      break;
    case 2:
      temp = (uint64_t)cpu.R[Rs] + (uint64_t)Rn;
      cpu.R[Rd] = (uint32_t)temp;
      SET_F(cpu.R[Rd] >> 31, cpu.R[Rd] == 0, (temp >> 32) != 0,
            V_ADD(cpu.R[Rn], Rn, cpu.R[Rd]))
      break;
    case 3:
      temp = (uint64_t)cpu.R[Rs] + imm;
      cpu.R[Rd] = (uint32_t)temp;
      SET_F(cpu.R[Rd] >> 31, cpu.R[Rd] == 0, (temp >> 32) != 0,
            V_ADD(cpu.R[Rn], (uint32_t)(imm - 1), cpu.R[Rd]))
      break;
    default:
      // Fuck CLion.
      break;
  }
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

#define MCI_PA                     \
  uint16_t Rd = (opcode >> 8) & 7; \
  uint16_t imm = opcode & 0xFF;

static inline void thumb_mov_reg_imm(uint16_t opcode) {
  MCI_PA
  cpu.R[Rd] = imm;
  SET_NZ(0, imm == 0)
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

static inline void thumb_cmp_reg_imm(uint16_t opcode) {
  MCI_PA
  uint64_t imm_ = (uint64_t)(uint32_t)(~imm) + 1;
  uint64_t temp = (uint64_t)cpu.R[Rd] + imm_;
  bool v = V_ADD(cpu.R[Rd], (uint32_t)(imm_ - 1), temp);
  SET_F(cpu.R[Rd] >> 31, cpu.R[Rd] == 0, (temp >> 32) != 0, v)
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

static inline void thumb_add_reg_imm(uint16_t opcode) {
  MCI_PA
  uint64_t temp = (uint64_t)cpu.R[Rd] + (uint64_t)imm;
  bool v = V_ADD(cpu.R[Rd], (uint32_t)(imm), temp);
  cpu.R[Rd] = (uint32_t)temp;
  SET_F(cpu.R[Rd] >> 31, cpu.R[Rd] == 0, (temp >> 32) != 0, v)
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

static inline void thumb_sub_reg_imm(uint16_t opcode) {
  MCI_PA
  uint64_t imm_ = (uint64_t)(uint32_t)(~imm) + 1;
  uint64_t temp = (uint64_t)cpu.R[Rd] + imm_;
  bool v = V_ADD(cpu.R[Rd], (uint32_t)(imm_ - 1), temp);
  cpu.R[Rd] = (uint32_t)temp;
  SET_F(cpu.R[Rd] >> 31, cpu.R[Rd] == 0, (temp >> 32) != 0, v)
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

DECL(move_compare_imm) {
  switch ((opcode >> 11) & 3) {
    case 0:
      thumb_mov_reg_imm(opcode);
      return;
    case 1:
      thumb_cmp_reg_imm(opcode);
      return;
    case 2:
      thumb_add_reg_imm(opcode);
      return;
    case 4:
      thumb_sub_reg_imm(opcode);
      return;
    default:
      // fuck CLion.
      break;
  }
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
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
  cpu.R14_svc = cpu.R[R_PC] + 2;
  cpu.SPSR_svc = cpu.CPSR;
  cpu_set_mode(MODE_SUPERVISOR);
  cpu.exec_mode = EXEC_ARM;
  cpu.CPSR.I = 1;
  cpu.CPSR.T = 0;
  cpu.CPSR.M = MODE_SUPERVISOR;
  cpu.R[R_PC] = 0x8;
  clocks -= get_access_cycles_nonseq16(cpu.R[R_PC]) +
            get_access_cycles_seq16(cpu.R[R_PC]);
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
    isSeq = (cpu.PC_old + 2 == cpu.R[R_PC]);
    cpu.PC_old = cpu.R[R_PC];
    register uint16_t opcode = memory_read_16(cpu.R[R_PC]);
    thumb_code[decode(opcode)](opcode);
    cpu.R[R_PC] += 2;
  }
}