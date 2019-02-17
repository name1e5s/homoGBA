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

#define LDM(ADDR, REG) cpu.R[REG] = memory_read_32((ADDR) & ~3)

#define STM(ADDR, REG) memory_write_32((ADDR) & -3, cpu.R[REG])

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

#define ALU_NZ(INSN, TARGET, BODY)                   \
  static inline void thumb_##INSN(uint16_t opcode) { \
    uint16_t Rs = (opcode >> 3) & 7;                 \
    uint16_t Rd = (opcode)&7;                        \
    BODY SET_NZ(BIT((TARGET), 31), (TARGET) == 0);   \
  }

#define ALU(INSN, BODY)                              \
  static inline void thumb_##INSN(uint16_t opcode) { \
    uint16_t Rs = (opcode >> 3) & 7;                 \
    uint16_t Rd = (opcode)&7;                        \
    BODY                                             \
  }

ALU_NZ(and, cpu.R[Rd], cpu.R[Rd] &= cpu.R[Rs];)
ALU_NZ(eor, cpu.R[Rd], cpu.R[Rd] ^= cpu.R[Rs];)
ALU_NZ(lsl,
       cpu.R[Rd],
       cpu.R[Rd] = lsl_by_reg(cpu.R[Rd], (uint8_t)(cpu.R[Rs] & 0xFF));)
ALU_NZ(lsr,
       cpu.R[Rd],
       cpu.R[Rd] = lsr_by_reg(cpu.R[Rd], (uint8_t)(cpu.R[Rs] & 0xFF));)
ALU_NZ(asr,
       cpu.R[Rd],
       cpu.R[Rd] = asr_by_reg(cpu.R[Rd], (uint8_t)(cpu.R[Rs] & 0xFF));)
ALU(adc,
    uint64_t temp = (uint64_t)cpu.R[Rd] + (uint64_t)cpu.R[Rs] + F_C ? 1ULL : 0;
    SET_F(BIT((uint32_t)temp, 31),
          temp == 0,
          (temp >> 32) != 0,
          V_ADD(cpu.R[Rd], cpu.R[Rs], (uint32_t)temp) != 0);

    cpu.R[Rd] = (uint32_t)temp;)
ALU(sbc,
    uint64_t temp = (uint64_t)cpu.R[Rd] + (uint64_t)~cpu.R[Rs] + F_C ? 1ULL : 0;
    SET_F(BIT((uint32_t)temp, 31),
          temp == 0,
          (temp >> 32) != 0,
          V_ADD(cpu.R[Rd], ~cpu.R[Rs], (uint32_t)temp));

    cpu.R[Rd] = (uint32_t)temp;)
ALU_NZ(ror,
       cpu.R[Rd],
       cpu.R[Rd] = ror_by_reg(cpu.R[Rd], (uint8_t)(cpu.R[Rs] & 0xFF));)
ALU_NZ(tst, temp, uint32_t temp = cpu.R[Rd] & cpu.R[Rs];)
ALU_NZ(neg, cpu.R[Rd], cpu.R[Rd] = (uint32_t)(-(int32_t)cpu.R[Rs]);
       F_C = (cpu.R[Rs] == 0);
       F_V = V_ADD(0, ~cpu.R[Rs], cpu.R[Rd]);)
ALU(cmp, uint64_t temp = (uint64_t)cpu.R[Rd] + (uint64_t)~cpu.R[Rs] + 1ULL;
    SET_F(BIT((uint32_t)temp, 31),
          temp == 0,
          (temp >> 32) != 0,
          V_ADD(cpu.R[Rd], ~cpu.R[Rs], (uint32_t)temp));)
ALU_NZ(cmn,
       (uint32_t)temp,
       uint64_t temp = (uint64_t)cpu.R[Rd] + (uint64_t)cpu.R[Rs];
       F_C = (temp & 0xFFFFFFFF00000000) != 0;
       F_V = (V_ADD(cpu.R[Rd], cpu.R[Rs], (uint32_t)temp) != 0);)
ALU_NZ(orr, cpu.R[Rd], cpu.R[Rd] |= cpu.R[Rs];)
ALU_NZ(mul, cpu.R[Rd], cpu.R[Rd] *= cpu.R[Rs];)
ALU_NZ(bic, cpu.R[Rd], cpu.R[Rd] &= ~cpu.R[Rs];)
ALU_NZ(mvn, cpu.R[Rd], cpu.R[Rd] = ~cpu.R[Rs];)

static inline int64_t thumb_mul_cycles(uint16_t opcode) {
  uint32_t data = cpu.R[opcode & 7];
  if (BIT(data, 31))
    data = ~data;
  if (data & 0xFF000000)
    return 4;
  if (data & 0x00FF0000)
    return 3;
  if (data & 0x0000FF00)
    return 2;
  return 1;
}

DECL(alu) {
  switch ((opcode >> 6) & 0xF) {
    case 0:
      thumb_and(opcode);
      break;
    case 1:
      thumb_eor(opcode);
      break;
    case 2:
      thumb_lsl(opcode);
      clocks -= 1;
      break;
    case 3:
      thumb_lsr(opcode);
      clocks -= 1;
      break;
    case 4:
      thumb_asr(opcode);
      clocks -= 1;
      break;
    case 5:
      thumb_adc(opcode);
      break;
    case 6:
      thumb_sbc(opcode);
      break;
    case 7:
      thumb_ror(opcode);
      clocks -= 1;
      break;
    case 8:
      thumb_tst(opcode);
      break;
    case 9:
      thumb_neg(opcode);
      break;
    case 0xA:
      thumb_cmp(opcode);
      break;
    case 0xB:
      thumb_cmn(opcode);
      break;
    case 0xC:
      thumb_orr(opcode);
      break;
    case 0xD:
      thumb_mul(opcode);
      clocks -= thumb_mul_cycles(opcode);
      break;
    case 0xE:
      thumb_bic(opcode);
      break;
    case 0xF:
      thumb_mvn(opcode);
      break;
    default:
      // fuck CLion.
      break;
  }
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

DECL(hi_reg_bx) {
  uint16_t Rd = (opcode & 7) | ((opcode >> 4) & 8);
  uint16_t Rs = (opcode >> 3) & 0xF;
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
  switch ((opcode >> 8) & 0x3) {
    case 0: {
      cpu.R[Rd] = cpu.R[Rd] + cpu.R[Rs] + (Rs == R_PC ? 4 : 0);
      if (Rd == R_PC) {
        clocks -= get_access_cycles_nonseq(1, cpu.R[R_PC]) +
                  get_access_cycles_seq(1, cpu.R[R_PC]);
        cpu.R[R_PC] = (cpu.R[R_PC] - 2) & ~1;
      }
    } break;
    case 1: {
      uint32_t Rd_val = cpu.R[Rd] + (Rd == R_PC ? 4 : 0);
      uint64_t t1 = (uint64_t) ~(cpu.R[Rs] + (Rs == R_PC ? 4 : 0));

      uint64_t temp = (uint64_t)Rd_val + t1 + 1ULL;
      SET_F(BIT((uint32_t)temp, 31), temp == 0, (temp >> 32) != 0,
            V_ADD(t1, (uint32_t)t1 - 1, (uint32_t)temp));
    } break;
    case 2: {
      cpu.R[Rd] = cpu.R[Rs] + (Rs == R_PC ? 4 : 0);
      if (Rd == R_PC) {
        clocks -= get_access_cycles_nonseq(1, cpu.R[R_PC]) +
                  get_access_cycles_seq(1, cpu.R[R_PC]);
        cpu.R[R_PC] = (cpu.R[R_PC] - 2) & ~1;
      }
    } break;
    case 3: {
      if (BIT(opcode, 7), opcode & 7)
        clocks -= 100;
      else {
        uint32_t offset = (Rs == R_PC ? (cpu.R[R_PC] + 4) & (~2) : cpu.R[Rs]);
        if ((offset & 1) == 0) {
          cpu.exec_mode = EXEC_ARM;
          cpu.CPSR.T = 0;
          cpu.R[R_PC] = offset & (~3);
          clocks -= get_access_cycles_nonseq(1, cpu.R[R_PC]) +
                    get_access_cycles_seq(1, cpu.R[R_PC]);
          cpu_run_arm(clocks);
          return;
        }
        cpu.R[R_PC] = offset & (~1) - 2;
        clocks -= get_access_cycles_nonseq(1, cpu.R[R_PC]) +
                  get_access_cycles_seq(1, cpu.R[R_PC]);
      }
    } break;
  }
}

DECL(load_pc_rel) {
  uint32_t offset = (uint32_t)((opcode & 0xFF) << 2);
  uint32_t Rd = (uint32_t)((opcode >> 8) & 3);

  uint32_t addr = (cpu.R[R_PC] + 4) & (~2) + offset;
  cpu.R[Rd] = memory_read_32(addr);
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]) +
            get_access_cycles_seq32(addr) + 1;
}

#define STR_(B)                                       \
  memory_write_##B(addr, (uint##B##_t)cpu.R[Rd]);     \
  clocks -= get_access_cycles_nonseq16(cpu.R[R_PC]) + \
            get_access_cycles_nonseq##B(addr);

#define LDR_(B)                                             \
  cpu.R[Rd] = (uint32_t)(uint##B##_t)memory_read_##B(addr); \
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]) +      \
            get_access_cycles_nonseq##B(addr) + 1;

DECL(load_store_reg) {
  uint16_t Rd = opcode & 7;
  uint16_t Rb = (opcode >> 3) & 7;
  uint16_t Ro = (opcode >> 6) & 7;
  uint32_t addr = cpu.R[Rd] + cpu.R[Ro];
  switch ((opcode >> 10) & 3) {
    case 0: {
      STR_(32)
    } break;
    case 1: {
      STR_(8)
    } break;
    case 2: {
      LDR_(32)
    } break;
    case 3: {
      LDR_(8);
    } break;
    default:
      // fuck CLion.
      break;
  }
}

DECL(load_store_se_half) {
  uint16_t Rd = opcode & 7;
  uint16_t Rb = (opcode >> 3) & 7;
  uint16_t Ro = (opcode >> 6) & 7;
  uint32_t addr = cpu.R[Rd] + cpu.R[Ro];
  switch ((opcode >> 10) & 3) {
    case 0: {
      STR_(16)
    } break;
    case 1: {
      cpu.R[Rd] = (uint32_t)(int32_t)(int8_t)(uint8_t)memory_read_8(addr);
      clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]) +
                get_access_cycles_nonseq16(addr) + 1;
    } break;
    case 2: {
      LDR_(16)
      if (addr & 1)
        cpu.R[Rd] = ror_by_imm(cpu.R[Rd], 8);
    } break;
    case 3: {
      if (addr & 1)
        cpu.R[Rd] = (uint32_t)(int32_t)(int8_t)memory_read_8(addr);
      else
        cpu.R[Rd] = (uint32_t)(int32_t)(int16_t)memory_read_16(addr);
      clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]) +
                get_access_cycles_nonseq16(addr) + 1;
    }
    default:
      // fuck CLion.
      break;
  }
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
  uint32_t offset = (uint32_t)((opcode & 0xFF) << 2);
  uint32_t Rd = (uint32_t)((opcode >> 8) & 3);
  if (BIT(opcode, 11))
    cpu.R[Rd] = cpu.R[R_SP] + offset;
  else
    cpu.R[Rd] = ((cpu.R[R_PC] + 4) & (~2)) + offset;
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

DECL(add_offset_sp) {
  int32_t offset = (opcode & 0x7F) << 2;
  if (BIT(opcode, 7))
    cpu.R[R_SP] -= offset;
  else
    cpu.R[R_SP] += offset;
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

static inline uint32_t thumb_popcnt(uint32_t val) {
  uint32_t count = 0;
  while (val != 0) {
    val &= val - 1;
    count++;
  }
  return count;
}

DECL(push_pop) {
  uint32_t regs = (uint32_t)(opcode & 0xFF);
  uint32_t bit = thumb_popcnt(regs);
  uint32_t addr = cpu.R[R_SP] - (bit << 2);
  if (BIT(opcode, 11)) {
    int up = 0;
    cpu.R[R_SP] = addr;
    if (BIT(opcode, 8))
      up = 16;
    else
      up = 8;
    for (int i = 0; i < up; i++) {
      if (BIT(regs, i)) {
        STM(addr, i);
        addr += 4;
      }
    }
    if (bit || BIT(opcode, 8))
      clocks -= get_access_cycles_seq32(addr) * (bit - 1) +
                get_access_cycles_nonseq32(addr);
    clocks -= get_access_cycles_nonseq16(cpu.R[R_PC]);
  } else {
    uint32_t count = 0;
    if (BIT(opcode, 8)) {
      for (int i = 0; i < 16; i++) {
        if (BIT(regs, i)) {
          LDM(addr, i);
          cpu.R[R_PC] += 4;
          count++;
        }
      }
      cpu.R[R_PC] = (cpu.R[R_PC] - 2) & (~1);
      clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]) + 1 +
                get_access_cycles_seq32(cpu.R[R_SP]) * (count - 1) +
                get_access_cycles_nonseq32(cpu.R[R_SP]) +
                get_access_cycles_nonseq16(cpu.R[R_PC]) +
                get_access_cycles_seq16(cpu.R[R_PC]);
    } else {
      for (int i = 0; i < 8; i++) {
        if (BIT(regs, i)) {
          LDM(addr, i);
          cpu.R[R_PC] += 4;
          count++;
        }
      }
      clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]) + 1;
      if (count)
        clocks -= get_access_cycles_seq32(cpu.R[R_SP]) * (count - 1) +
                  get_access_cycles_nonseq32(cpu.R[R_SP]);
    }
  }
}

DECL(multiple_load_store) {
  // TODO:
}

static inline bool thumb_cond(uint16_t opcode) {
  switch ((opcode >> 8) & 0xF) {
    case 0:
      return COND(EQ);
    case 1:
      return COND(NE);
    case 2:
      return COND(CS);
    case 3:
      return COND(CC);
    case 4:
      return COND(MI);
    case 5:
      return COND(PL);
    case 6:
      return COND(VS);
    case 7:
      return COND(VC);
    case 8:
      return COND(HI);
    case 9:
      return COND(LS);
    case 0xA:
      return COND(GE);
    case 0xB:
      return COND(LT);
    case 0xC:
      return COND(GT);
    case 0xD:
      return COND(LE);
    default:
      return false;
  }
}

DECL(uncond_branch) {
  clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
  int32_t offset = (opcode & 0x3FF) << 1;
  if (BIT(opcode, 10))
    offset |= 0xFFFFF800;
  cpu.R[R_PC] += offset + 4;
  clocks -= get_access_cycles_nonseq16(cpu.R[R_PC]) +
            get_access_cycles_seq16(cpu.R[R_PC]);
  cpu.R[R_PC] -= 2;
}

DECL(cond_branch) {
  if (thumb_cond(opcode))
    thumb_uncond_branch((uint16_t)(opcode & 0x00FF));
  else
    clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
}

DECL(long_branch) {
  if (BIT(opcode, 11)) {
    uint32_t offset = ((uint32_t)opcode & 0x7FF) << 12;
    if (BIT(opcode, 10))
      offset |= 0xFF800000;
    cpu.R[R_LR] = cpu.R[R_PC] + 4 + offset;
    clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
  } else {
    clocks -= get_access_cycles(isSeq, 0, cpu.R[R_PC]);
    uint32_t temp = cpu.R[R_LR] + (((uint32_t)opcode & 0x7FF) << 1);
    cpu.R[R_LR] = (cpu.R[R_PC] + 2) | 0x00000001;
    cpu.R[R_PC] = temp;
    clocks -= get_access_cycles_nonseq16(cpu.R[R_PC]) +
              get_access_cycles_seq16(cpu.R[R_PC]);
  }
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