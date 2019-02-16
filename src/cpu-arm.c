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

#define DECL(BLOCK) static void arm_##BLOCK(uint32_t opcode)
typedef void (*arm_block)(uint32_t opcode);

bool isSeq;

DECL(branch_and_exchange) {
  uint32_t Rn = opcode & 0xF;
  if (BIT(opcode, 5))
    cpu.R[R_LR] = cpu.R[R_PC] + 4;

  if (cpu.R[Rn] & 0x1) {
    cpu.exec_mode = EXEC_THUMB;
    cpu.CPSR.T = 1;
    cpu.R[R_PC] = (cpu.R[Rn] + (Rn == R_PC ? 8 : 0)) & (~1);
    clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) * 2 +
              get_access_cycles_nonseq32(cpu.R[R_PC]);
    cpu_run_thumb(clocks);
  }

  cpu.R[R_PC] = (cpu.R[Rn] + (Rn == R_PC ? 8 : 0)) & (~3) - 4;
  clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) * 2 +
            get_access_cycles_nonseq32(cpu.R[R_PC]);
}

DECL(branch_with_link) {
  uint32_t nn = opcode & 0x00FFFFFF;
  if (BIT(nn, 23))
    nn |= 0xFF000000;
  if (BIT(opcode, 24))
    cpu.R[R_LR] = cpu.R[R_PC] + 4;
  cpu.R[R_PC] += 8 + (nn << 2) - 4;
  clocks -= (get_access_cycles(isSeq, 1, cpu.PC_old) +
             get_access_cycles_seq32(cpu.R[R_PC]) +
             get_access_cycles_nonseq32(cpu.R[R_PC]));
}
// ALU Instructions.
#define ALU_3OP_ELEM(SHIFTNAME, SHIFT, INSN, N, Z, C, V, CARRY, PRE_BODY, \
                     BODY, NUM, ...)                                      \
  static inline void arm_##INSN##_##SHIFTNAME(uint32_t opcode) {          \
    uint32_t Rd = (opcode >> 12) & 0xF;                                   \
    uint32_t Rn = (opcode >> 16) & 0xF;                                   \
    uint32_t Rm = opcode & 0xF;                                           \
    uint32_t Rs = (opcode >> 8) & 0xF;                                    \
    uint32_t shift = (opcode >> 5) & 0x3;                                 \
    uint32_t val = opcode & 0xFF;                                         \
    uint32_t value = (opcode >> 7) & 0x1F;                                \
    uint32_t ror = (opcode >> 7) & 0x1E;                                  \
    PRE_BODY                                                              \
    uint32_t Rn_val = cpu.R[Rn];                                          \
    uint32_t Rm_val = cpu.R[Rm];                                          \
    uint32_t Rs_val = cpu.R[Rs];                                          \
    if (Rn == R_PC)                                                       \
      Rn_val += NUM;                                                      \
    if (Rm == R_PC)                                                       \
      Rm_val += NUM;                                                      \
    uint32_t imm = SHIFT(__VA_ARGS__);                                    \
    BODY F_N = (N);                                                       \
    F_Z = (Z);                                                            \
    F_C = (C);                                                            \
    F_V = (V);                                                            \
    if (Rd == R_PC)                                                       \
      cpu.R[Rd] -= 4;                                                     \
  }                                                                       \
  static inline void arm_##INSN##s_##SHIFTNAME(uint32_t opcode) {         \
    uint32_t Rd = (opcode >> 12) & 0xF;                                   \
    uint32_t Rn = (opcode >> 16) & 0xF;                                   \
    uint32_t Rm = opcode & 0xF;                                           \
    uint32_t Rs = (opcode >> 8) & 0xF;                                    \
    uint32_t shift = (opcode >> 5) & 0x3;                                 \
    uint32_t val = opcode & 0xFF;                                         \
    uint32_t ror = (opcode >> 7) & 0x1E;                                  \
    uint32_t value = (opcode >> 7) & 0x1F;                                \
    PRE_BODY                                                              \
    uint32_t Rn_val = cpu.R[Rn];                                          \
    uint32_t Rm_val = cpu.R[Rm];                                          \
    uint32_t Rs_val = cpu.R[Rs];                                          \
    if (Rn == R_PC)                                                       \
      Rn_val += NUM;                                                      \
    if (Rm == R_PC)                                                       \
      Rm_val += NUM;                                                      \
    uint32_t imm = SHIFT##CARRY(__VA_ARGS__);                             \
    BODY F_N = (N);                                                       \
    F_Z = (Z);                                                            \
    F_C = (C);                                                            \
    F_V = (V);                                                            \
    if (Rd == R_PC) {                                                     \
      if (cpu.CPSR.M != MODE_USER) {                                      \
        cpu.CPSR = cpu.SPSR;                                              \
        cpu_set_mode(cpu.SPSR.M);                                         \
      }                                                                   \
      cpu.R[Rd] -= 4;                                                     \
    }                                                                     \
  }

#define ALU_3OP_BLOCK(INSN, N, Z, C, V, CARRY, PRE_BODY, BODY)                 \
  ALU_3OP_ELEM(imm, build_imm, INSN, N, Z, C, V, CARRY, PRE_BODY, BODY, 8,     \
               val, ror)                                                       \
  ALU_3OP_ELEM(rsr, cpu_shift_by_reg, INSN, N, Z, C, V, CARRY, PRE_BODY, BODY, \
               12, shift, Rm_val, Rs_val)                                      \
  ALU_3OP_ELEM(rsi, cpu_shift_by_imm, INSN, N, Z, C, V, CARRY, PRE_BODY, BODY, \
               15, shift, Rm_val, value)

#define ALU_3OP(INSN)                                    \
  do {                                                   \
    arm_##INSN(opcode);                                  \
    clocks -= get_access_cycles(isSeq, 1, cpu.R[R_PC]);  \
    if (((opcode >> 12) & 0xF) == 0xF) {                 \
      clocks -= get_access_cycles_seq32(cpu.R[R_PC]) +   \
                get_access_cycles_nonseq32(cpu.R[R_PC]); \
      if (cpu.CPSR.T == 1) {                             \
        cpu.exec_mode = EXEC_THUMB;                      \
        cpu.R[R_PC] += 4;                                \
        return cpu_run_thumb(clocks);                    \
      }                                                  \
    }                                                    \
  } while (0)

ALU_3OP_BLOCK(and,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              F_C,
              F_V,
              _carry,
              ,
              cpu.R[Rd] = Rn_val & imm;)
ALU_3OP_BLOCK(eor,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              F_C,
              F_V,
              _carry,
              ,
              cpu.R[Rd] = Rn_val ^ imm;)
ALU_3OP_BLOCK(sub,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              (tmp >> 32) != 0,
              V_ADD(Rn_val, (uint32_t)(uint64_t)(~imm), cpu.R[Rd]) != 0,
              ,
              ,
              uint64_t tmp = (uint64_t)Rn_val + (uint64_t)(~imm) + 1ULL;
              cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_BLOCK(rsb,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              (tmp >> 32) != 0,
              V_ADD(imm, (uint32_t)(uint64_t)(~Rn_val), cpu.R[Rd]) != 0,
              ,
              ,
              uint64_t tmp = (uint64_t)(~Rn_val) + (uint64_t)(imm) + 1ULL;
              cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_BLOCK(add,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              (tmp >> 32) != 0,
              V_ADD(Rn_val, imm, cpu.R[Rd]) != 0,
              ,
              ,
              uint64_t tmp = (uint64_t)(Rn_val) + (uint64_t)(imm);
              cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_BLOCK(adc,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              (tmp >> 32) != 0,
              V_ADD(Rn_val, imm, cpu.R[Rd]) != 0,
              ,
              uint64_t prev_c = F_C;
              , uint64_t tmp = (uint64_t)(Rn_val) + (uint64_t)(imm) + prev_c;
              cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_BLOCK(sbc,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              (tmp >> 32) != 0,
              V_ADD(Rn_val, (uint32_t)(uint64_t)(~imm), (uint32_t)tmp) != 0,
              ,
              uint64_t prev_c = F_C;
              ,
              uint64_t tmp = (uint64_t)Rn_val + (uint64_t)(~imm) + 1ULL -
                             prev_c;
              cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_BLOCK(rsc,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              (tmp >> 32) != 0,
              V_ADD(imm, (uint32_t)(uint64_t)(~Rn_val), (uint32_t)tmp) != 0,
              ,
              uint64_t prev_c = F_C;
              ,
              uint64_t tmp = (uint64_t)(~Rn_val) + (uint64_t)(imm) + 1ULL -
                             prev_c;
              cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_BLOCK(orr,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              F_C,
              F_V,
              _carry,
              ,
              cpu.R[Rd] = Rn_val | imm;)
ALU_3OP_BLOCK(bic,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              F_C,
              F_V,
              _carry,
              ,
              cpu.R[Rd] = Rn_val & (~imm);)
ALU_3OP_BLOCK(mov,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              F_C,
              F_V,
              _carry,
              ,
              cpu.R[Rd] = imm;)
ALU_3OP_BLOCK(mvn,
              BIT(cpu.R[Rd], 31),
              cpu.R[Rd] == 0,
              F_C,
              F_V,
              _carry,
              ,
              cpu.R[Rd] = ~imm;)

#define ALU_2OP_ELEM(SHIFTNAME, SHIFT, INSN, N, Z, C, V, CARRY, PRE_BODY, \
                     BODY, NUM, ...)                                      \
  static inline void arm_##INSN##_##SHIFTNAME(uint32_t opcode) {          \
    uint32_t R = (opcode >> 16) & 0xF;                                    \
    uint32_t Rm = opcode & 0xF;                                           \
    uint32_t Rs = (opcode >> 8) & 0xF;                                    \
    uint32_t shift = (opcode >> 5) & 0x3;                                 \
    uint32_t val = (opcode & 0xFF);                                       \
    uint32_t ror = (opcode >> 7) & 0x1E;                                  \
    uint32_t value = (opcode >> 7) & 0x1F;                                \
    PRE_BODY                                                              \
    uint32_t R_val = cpu.R[R];                                            \
    uint32_t Rm_val = cpu.R[Rm] + (Rm == R_PC ? NUM : 0);                 \
    uint32_t Rs_val = cpu.R[Rs];                                          \
    uint32_t imm = SHIFT##CARRY(__VA_ARGS__);                             \
    if (R == R_PC)                                                        \
      R_val += NUM;                                                       \
    BODY F_N = (N);                                                       \
    F_Z = (Z);                                                            \
    F_C = (C);                                                            \
    F_V = (V);                                                            \
  }

#define ALU_2OP_BLOCK(INSN, N, Z, C, V, CARRY, PRE_BODY, BODY)                 \
  ALU_2OP_ELEM(imm, build_imm, INSN, N, Z, C, V, CARRY, PRE_BODY, BODY, 8,     \
               val, ror)                                                       \
  ALU_2OP_ELEM(rsr, cpu_shift_by_reg, INSN, N, Z, C, V, CARRY, PRE_BODY, BODY, \
               12, shift, Rm_val, Rs_val)                                      \
  ALU_2OP_ELEM(rsi, cpu_shift_by_imm, INSN, N, Z, C, V, CARRY, PRE_BODY, BODY, \
               15, shift, Rm_val, value)

#define ALU_2OP(INSN)                                   \
  do {                                                  \
    arm_##INSN(opcode);                                 \
    clocks -= get_access_cycles(isSeq, 1, cpu.R[R_PC]); \
  } while (0)

ALU_2OP_BLOCK(tst,
              BIT(tmp, 31),
              tmp == 0,
              F_C,
              F_V,
              _carry,
              ,
              uint32_t tmp = R_val & imm;)
ALU_2OP_BLOCK(teq,
              BIT(tmp, 31),
              tmp == 0,
              F_C,
              F_V,
              _carry,
              ,
              uint32_t tmp = R_val ^ imm;)

ALU_2OP_BLOCK(cmp,
              BIT(temp, 31),
              temp == 0,
              (tmp >> 32) != 0,
              V_ADD(R_val, (uint32_t)(uint64_t)(~imm), temp) != 0,
              ,
              ,
              uint64_t tmp = (uint64_t)R_val + (uint64_t)(~imm) + 1ULL;
              uint32_t temp = (uint32_t)tmp;)
ALU_2OP_BLOCK(cmn,
              BIT(temp, 31),
              temp == 0,
              (tmp >> 32) != 0,
              V_ADD(R_val, imm, temp) != 0,
              ,
              ,
              uint64_t tmp = (uint64_t)(R_val) + (uint64_t)(imm);
              uint32_t temp = (uint32_t)tmp;)

#define ALU_BLOCK(TYPE, S)        \
  switch ((opcode >> 21) & 0xF) { \
    case 0x0:                     \
      ALU_3OP(and##S##_##TYPE);   \
      break;                      \
    case 0x1:                     \
      ALU_3OP(eor##S##_##TYPE);   \
      break;                      \
    case 0x2:                     \
      ALU_3OP(sub##S##_##TYPE);   \
      break;                      \
    case 0x3:                     \
      ALU_3OP(rsb##S##_##TYPE);   \
      break;                      \
    case 0x4:                     \
      ALU_3OP(add##S##_##TYPE);   \
      break;                      \
    case 0x5:                     \
      ALU_3OP(adc##S##_##TYPE);   \
      break;                      \
    case 0x6:                     \
      ALU_3OP(sbc##S##_##TYPE);   \
      break;                      \
    case 0x7:                     \
      ALU_3OP(rsc##S##_##TYPE);   \
      break;                      \
    case 0x8:                     \
      ALU_2OP(tst_##TYPE);        \
      break;                      \
    case 0x9:                     \
      ALU_2OP(teq_##TYPE);        \
      break;                      \
    case 0xA:                     \
      ALU_2OP(cmp_##TYPE);        \
      break;                      \
    case 0xB:                     \
      ALU_2OP(cmn_##TYPE);        \
      break;                      \
    case 0xC:                     \
      ALU_3OP(orr##S##_##TYPE);   \
      break;                      \
    case 0xD:                     \
      ALU_3OP(mov##S##_##TYPE);   \
      break;                      \
    case 0xE:                     \
      ALU_3OP(bic##S##_##TYPE);   \
      break;                      \
    case 0xF:                     \
      ALU_3OP(mvn##S##_##TYPE);   \
      break;                      \
    default:                      \
      break;                      \
  }

DECL(data_processing) {
  switch (((BIT(opcode, 25) << 1) | (BIT(opcode, 20)))) {
    case 0:
      ALU_BLOCK(imm, )
      break;
    case 1:
      ALU_BLOCK(imm, s)
      break;
    case 2:
      if (BIT(opcode, 4)) {
        ALU_BLOCK(rsr, )
      } else {
        ALU_BLOCK(rsi, )
      }
      break;
    case 3:
      if (BIT(opcode, 4)) {
        ALU_BLOCK(rsr, s)
      } else {
        ALU_BLOCK(rsi, s)
      }
      break;
    default:
      // Fuck CLion.
      break;
  }
}

DECL(psr) {
  switch ((opcode >> 20) & 0xFF) {
      // MRS
    case 0x10:
      cpu.R[(opcode >> 12) & 0xF] = cpsr_to_uint(cpu.CPSR);
      break;
    case 0x14:
      cpu.R[(opcode >> 12) & 0xF] = cpsr_to_uint(cpu.SPSR);
      break;
      // MSR
    case 0x12: {
      uint32_t data = cpu.R[opcode & 0xF];
      uint32_t res = 0;
      if (BIT(opcode, 19)) {
        res |= data & 0xFF000000;
        cpu.CPSR = uint_to_cpsr(cpsr_to_uint(cpu.CPSR) & 0x00FFFFFF);
      }
      if (cpu.CPSR.M != MODE_USER) {
        if (BIT(opcode, 18)) {
          res |= data & 0x00FF0000;
          cpu.CPSR = uint_to_cpsr(cpsr_to_uint(cpu.CPSR) & 0XFF00FFFF);
        }
        if (BIT(opcode, 17)) {
          res |= data & 0x0000FF00;
          cpu.CPSR = uint_to_cpsr(cpsr_to_uint(cpu.CPSR) & 0xFFFF00FF);
        }
        if (BIT(opcode, 16)) {
          res |= data & 0x000000FF;
          cpu.CPSR = uint_to_cpsr(cpsr_to_uint(cpu.CPSR) & 0xFFFFFF00);
          cpu.CPSR = uint_to_cpsr(cpsr_to_uint(cpu.CPSR) | res);
          cpu_set_mode(res & 0x1F);
        }
      } else
        cpu.CPSR = uint_to_cpsr(cpsr_to_uint(cpu.CPSR) | res);
      break;
    }
    case 0x16: {
      uint32_t data = cpu.R[opcode & 0xF];
      uint32_t res = 0;
      if (BIT(opcode, 19)) {
        res |= data & 0xFF000000;
        cpu.SPSR = uint_to_cpsr(cpsr_to_uint(cpu.SPSR) & 0x00FFFFFF);
      }
      if (cpu.SPSR.M != MODE_USER) {
        if (BIT(opcode, 18)) {
          res |= data & 0x00FF0000;
          cpu.SPSR = uint_to_cpsr(cpsr_to_uint(cpu.SPSR) & 0XFF00FFFF);
        }
        if (BIT(opcode, 17)) {
          res |= data & 0x0000FF00;
          cpu.SPSR = uint_to_cpsr(cpsr_to_uint(cpu.SPSR) & 0xFFFF00FF);
        }
        if (BIT(opcode, 16)) {
          res |= data & 0x000000FF;
          cpu.SPSR = uint_to_cpsr(cpsr_to_uint(cpu.SPSR) & 0xFFFFFF00);
          cpu.SPSR = uint_to_cpsr(cpsr_to_uint(cpu.SPSR) | res);
          cpu_set_mode(res & 0x1F);
        }
      } else
        cpu.SPSR = uint_to_cpsr(cpsr_to_uint(cpu.SPSR) | res);
      break;
    }
    default:
      // Fuck CLion.
      break;
  }
  clocks -= get_access_cycles(isSeq, 1, cpu.R[R_PC]);
}

#define MUL_SET_FLAG             \
  cpu.CPSR.Z = (cpu.R[Rd] == 0); \
  cpu.CPSR.N = BIT(cpu.R[Rd], 31);

static inline void arm_mul(uint32_t opcode) {
  uint32_t Rd = (opcode >> 16) & 0xF;
  uint32_t Rs = (opcode >> 8) & 0xF;
  uint32_t Rm = opcode & 0xF;
  cpu.R[Rd] = (uint32_t)((int32_t)cpu.R[Rs] * (int32_t)cpu.R[Rm]);
}

static inline void arm_muls(uint32_t opcode) {
  uint32_t Rd = (opcode >> 16) & 0xF;
  uint32_t Rs = (opcode >> 8) & 0xF;
  uint32_t Rm = opcode & 0xF;
  cpu.R[Rd] = (uint32_t)((int32_t)cpu.R[Rs] * (int32_t)cpu.R[Rm]);
  MUL_SET_FLAG
}

static inline void arm_mla(uint32_t opcode) {
  uint32_t Rd = (opcode >> 16) & 0xF;
  uint32_t Rn = (opcode >> 12) & 0xF;
  uint32_t Rs = (opcode >> 8) & 0xF;
  uint32_t Rm = opcode & 0xF;
  cpu.R[Rd] =
      (uint32_t)((int32_t)cpu.R[Rs] * (int32_t)cpu.R[Rm] + (int32_t)cpu.R[Rn]);
}

static inline void arm_mlas(uint32_t opcode) {
  uint32_t Rd = (opcode >> 16) & 0xF;
  uint32_t Rn = (opcode >> 12) & 0xF;
  uint32_t Rs = (opcode >> 8) & 0xF;
  uint32_t Rm = opcode & 0xF;
  cpu.R[Rd] =
      (uint32_t)((int32_t)cpu.R[Rs] * (int32_t)cpu.R[Rm] + (int32_t)cpu.R[Rn]);
  MUL_SET_FLAG
}

#define MULL(INSN, EXPR)                              \
  static inline void arm_##INSN(uint32_t opcode) {    \
    uint32_t RdHi = (opcode >> 16) & 0xF;             \
    uint32_t RdLo = (opcode >> 12) & 0xF;             \
    uint32_t Rs = (opcode >> 8) & 0xF;                \
    uint32_t Rm = opcode & 0xF;                       \
    uint64_t tmp = (uint64_t)(EXPR);                  \
                                                      \
    cpu.R[RdHi] = (uint32_t)(tmp >> 32);              \
    cpu.R[RdLo] = (uint32_t)(tmp);                    \
  }                                                   \
  static inline void arm_##INSN##s(uint32_t opcode) { \
    uint32_t RdHi = (opcode >> 16) & 0xF;             \
    uint32_t RdLo = (opcode >> 12) & 0xF;             \
    uint32_t Rs = (opcode >> 8) & 0xF;                \
    uint32_t Rm = opcode & 0xF;                       \
    uint64_t tmp = (uint64_t)(EXPR);                  \
                                                      \
    cpu.R[RdHi] = (uint32_t)(tmp >> 32);              \
    cpu.R[RdLo] = (uint32_t)(tmp);                    \
    cpu.CPSR.Z = (tmp == 0);                          \
    cpu.CPSR.N = BIT(cpu.R[RdHi], 31);                \
  }

MULL(umull, (uint64_t)cpu.R[Rs] * (uint64_t)cpu.R[Rm])
MULL(umlal,
     (((uint64_t)cpu.R[Rm]) * ((uint64_t)cpu.R[Rs])) +
         ((((uint64_t)cpu.R[RdHi]) << 32) | (uint64_t)cpu.R[RdLo]))
MULL(smull,
     (int64_t)((int64_t)(int32_t)cpu.R[Rm] * (int64_t)(int32_t)cpu.R[Rs]))
MULL(smlal,
     (((int64_t)(int32_t)cpu.R[Rm]) * ((int64_t)(int32_t)cpu.R[Rs])) +
         ((((uint64_t)cpu.R[RdHi]) << 32) | (uint64_t)cpu.R[RdLo]))

static inline int64_t signed_mul_cycles(uint32_t opcode) {
  uint32_t data = cpu.R[(opcode >> 16) & 0xF];
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

static inline int64_t unsigned_mul_cycles(uint32_t opcode) {
  uint32_t data = cpu.R[(opcode >> 16) & 0xF];
  if (data & 0xFF000000)
    return 4;
  if (data & 0x00FF0000)
    return 3;
  if (data & 0x0000FF00)
    return 2;
  return 1;
}

#define MUL_BLOCK(INSN, CLOCK) \
  do {                         \
    if (BIT(opcode, 20))       \
      arm_##INSN##s(opcode);   \
    else                       \
      arm_##INSN(opcode);      \
    clocks -= CLOCK;           \
  } while (0)

DECL(multiply) {
  switch ((opcode >> 21) & 0xF) {
    case 0:
      MUL_BLOCK(mul, get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
                         signed_mul_cycles(opcode));
      break;
    case 1:
      MUL_BLOCK(mla, get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
                         signed_mul_cycles(opcode) + 1);
      break;
    case 4:
      MUL_BLOCK(umull, get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
                           unsigned_mul_cycles(opcode) + 1);
      break;
    case 5:
      MUL_BLOCK(umlal, get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
                           unsigned_mul_cycles(opcode) + 2);
      break;
    case 6:
      MUL_BLOCK(smull, get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
                           signed_mul_cycles(opcode) + 1);
      break;
    case 7:
      MUL_BLOCK(smlal, get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
                           signed_mul_cycles(opcode) + 2);
      break;
    default:  // UNDEF
      cpu.R[R_PC] -= 4;
      clocks -= 100;
      break;
  }
}

#define SG_PC                                           \
  if (Rd == R_PC) {                                     \
    clocks -= get_access_cycles_nonseq32(cpu.R[R_PC]) + \
              get_access_cycles_seq32(cpu.R[R_PC]);     \
    cpu.R[R_PC] -= 4;                                   \
  }

DECL(single_transfer) {
  uint32_t Rn = ((opcode >> 16) & 0xF);
  uint32_t Rd = ((opcode >> 12) & 0xF);
  uint32_t Rn_val = cpu.R[Rn] + (Rn == R_PC ? 8 : 0);
  uint32_t Rd_val = cpu.R[Rn] + (Rn == R_PC ? 12 : 0);
  int32_t offset = 0;
  if (BIT(opcode, 25))
    offset = cpu_shift_by_imm((opcode >> 5) & 0x3, cpu.R[opcode & 0xF],
                              (uint8_t)(opcode >> 7) & 0x1F);
  else
    offset = opcode & 0x00000FFF;

  switch ((opcode >> 20) & 0xFF) {
    case 0x00:
    case 0x02:
      memory_write_32(Rn_val, Rd_val);
      cpu.R[Rn] = Rn_val - offset;
      clocks -= 2 * get_access_cycles_nonseq32(Rn_val);
      break;
    case 0x01:
    case 0x03:
      cpu.R[Rn] = Rn_val - offset;
      cpu.R[Rd] = memory_read_32(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq32(Rn_val) + 1;
      SG_PC
      break;
    case 0x04:
    case 0x06:
      memory_write_8(Rn_val, (uint8_t)Rd_val);
      cpu.R[Rn] = Rn_val - offset;
      clocks -= 2 * get_access_cycles_nonseq16(Rn_val);
      break;
    case 0x05:
    case 0x07:
      cpu.R[Rn] = Rn_val - offset;
      cpu.R[Rd] = memory_read_8(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq16(Rn_val) + 1;
      SG_PC
      break;
    case 0x08:
    case 0x0A:
      memory_write_32(Rn_val, Rd_val);
      cpu.R[Rn] = Rn_val + offset;
      clocks -= 2 * get_access_cycles_nonseq32(Rn_val);
      break;
    case 0x09:
    case 0x0B:
      cpu.R[Rn] = Rn_val + offset;
      cpu.R[Rd] = memory_read_32(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq32(Rn_val) + 1;
      SG_PC
      break;
    case 0x0C:
    case 0x0E:
      memory_write_8(Rn_val, (uint8_t)Rd_val);
      cpu.R[Rn] = Rn_val + offset;
      clocks -= 2 * get_access_cycles_nonseq16(Rn_val);
      break;
    case 0x0D:
    case 0x0F:
      cpu.R[Rn] = Rn_val + offset;
      cpu.R[Rd] = memory_read_8(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq16(Rn_val) + 1;
      SG_PC
      break;
    case 0x10:
      Rn_val -= offset;
      memory_write_32(Rn_val, Rd_val);
      clocks -= 2 * get_access_cycles_nonseq32(Rn_val);
      break;
    case 0x11:
      Rn_val -= offset;
      cpu.R[Rd] = memory_read_32(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq32(Rn_val) + 1;
      SG_PC
      break;
    case 0x12:
      Rn_val -= offset;
      cpu.R[Rn] = Rn_val;
      memory_write_32(Rn_val, Rd_val);
      clocks -= 2 * get_access_cycles_nonseq32(Rn_val);
      break;
    case 0x13:
      Rn_val -= offset;
      cpu.R[Rn] = Rn_val;
      cpu.R[Rd] = memory_read_32(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq32(Rn_val) + 1;
      SG_PC
      break;
    case 0x14:
      Rn_val -= offset;
      memory_write_8(Rn_val, (uint8_t)Rd_val);
      clocks -= 2 * get_access_cycles_nonseq16(Rn_val);
      break;
    case 0x15:
      Rn_val -= offset;
      cpu.R[Rd] = memory_read_8(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq16(Rn_val) + 1;
      SG_PC
      break;
    case 0x16:
      Rn_val -= offset;
      cpu.R[Rn] = Rn_val;
      memory_write_8(Rn_val, (uint8_t)Rd_val);
      clocks -= 2 * get_access_cycles_nonseq16(Rn_val);
      break;
    case 0x17:
      Rn_val -= offset;
      cpu.R[Rn] = Rn_val;
      cpu.R[Rd] = memory_read_8(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq16(Rn_val) + 1;
      SG_PC
      break;
    case 0x18:
      Rn_val += offset;
      memory_write_32(Rn_val, Rd_val);
      clocks -= 2 * get_access_cycles_nonseq32(Rn_val);
      break;
    case 0x19:
      Rn_val += offset;
      cpu.R[Rd] = memory_read_32(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq32(Rn_val) + 1;
      SG_PC
      break;
    case 0x1A:
      Rn_val += offset;
      cpu.R[Rn] = Rn_val;
      memory_write_32(Rn_val, Rd_val);
      clocks -= 2 * get_access_cycles_nonseq32(Rn_val);
      break;
    case 0x1B:
      Rn_val += offset;
      cpu.R[Rn] = Rn_val;
      cpu.R[Rd] = memory_read_32(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq32(Rn_val) + 1;
      SG_PC
      break;
    case 0x1C:
      Rn_val += offset;
      memory_write_8(Rn_val, (uint8_t)Rd_val);
      clocks -= 2 * get_access_cycles_nonseq16(Rn_val);
      break;
    case 0x1D:
      Rn_val += offset;
      cpu.R[Rd] = memory_read_8(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq16(Rn_val) + 1;
      SG_PC
      break;
    case 0x1E:
      Rn_val += offset;
      cpu.R[Rn] = Rn_val;
      memory_write_8(Rn_val, (uint8_t)Rd_val);
      clocks -= 2 * get_access_cycles_nonseq16(Rn_val);
      break;
    case 0x1F:
      Rn_val += offset;
      cpu.R[Rn] = Rn_val;
      cpu.R[Rd] = memory_read_8(Rn_val);
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq16(Rn_val) + 1;
      SG_PC
      break;
    default:
      // Fuck CLion.
      break;
  }
}

DECL(half_transfer) {
  uint32_t Rn = (opcode >> 16) & 0xF;
  uint32_t Rd = (opcode >> 12) & 0xF;

  uint32_t offset = 0;
  uint32_t addr = cpu.R[Rn];
  uint32_t val = 0;

  if (BIT(opcode, 22))
    offset = ((opcode >> 8) & 0xF) << 4 | (opcode & 0xF);
  else
    offset = cpu.R[opcode & 0xF];

  if (BIT(opcode, 24)) {
    if (BIT(opcode, 23))
      addr += offset;
    else
      addr -= offset;
  }

  switch ((opcode >> 5) & 0x3) {
    case 1:
      if (BIT(opcode, 20)) {
        val = memory_read_16(addr);
        cpu.R[Rd] = val;
        if (Rd == R_PC)
          clocks -= get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
                    get_access_cycles_nonseq32(cpu.R[R_PC]);

        clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                  get_access_cycles_nonseq16(addr);

      } else {
        val = cpu.R[Rd];
        if (Rd == R_PC) {
          val += 4;
        }
        memory_write_32(addr, val & 0x0000FFFF);
        clocks -= 2 * get_access_cycles_nonseq16(addr);
      }
      break;
    case 2:
      val = memory_read_8(addr);
      if (BIT(val, 7))
        val |= 0xFFFFFF00;

      cpu.R[Rd] = val;
      if (Rd == R_PC)
        clocks -= get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
                  get_access_cycles_nonseq32(cpu.R[R_PC]);

      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq16(addr);
      break;
    case 3:
      val = memory_read_16(addr);
      if (BIT(val, 15))
        val |= 0xFFFF0000;

      cpu.R[Rd] = val;
      if (Rd == R_PC)
        clocks -= get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
                  get_access_cycles_nonseq32(cpu.R[R_PC]);

      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_nonseq16(addr);
      break;
    default:
      return;
  }
  if (!BIT(opcode, 24)) {
    if (BIT(opcode, 23))
      addr += offset;
    else
      addr -= offset;
  }
  if (BIT(opcode, 21) && Rd != Rn)
    cpu.R[Rn] = addr;
}

static inline int popcnt(uint32_t val) {
  int count = 0;
  while (val != 0) {
    val &= val - 1;
    count++;
  }
  return count;
}

#define BT_PA                         \
  uint32_t Rn = (opcode >> 16) & 0xF; \
  uint32_t addr = cpu.R[Rn];          \
  uint32_t bit = popcnt(opcode & 0x0000FFFF);

#define LDM(ADDR, REG) cpu.R[REG] = memory_read_32((ADDR) & ~3)

#define STM(ADDR, REG) \
  memory_write_32((ADDR) & -3, (cpu.R[REG] + ((REG) == R_PC ? 12 : 0)))

#define STM_CLK                                       \
  clocks -= get_access_cycles_nonseq32(cpu.R[R_PC]) + \
            get_access_cycles_nonseq32(addr) +        \
            get_access_cycles_seq32(addr) * (bit - 1);

#define LDM_CLK                                         \
  if (BIT(opcode, 15)) {                                \
    clocks -= get_access_cycles_nonseq32(cpu.R[R_PC]) + \
              get_access_cycles_seq32(cpu.R[R_PC]);     \
    cpu.R[R_PC] -= 4;                                   \
  } else                                                \
    clocks -= get_access_cycles_nonseq32(addr) +        \
              get_access_cycles_seq32(addr) * (bit - 1) + 1;

DECL(block_transfer) {
  switch ((opcode >> 20) & 0xFF) {
    case 0x00: {
      BT_PA
      addr -= bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          STM(addr, i);
        }
      STM_CLK
    } break;
    case 0x02: {
      BT_PA
      addr -= bit << 2;
      bool flag = true;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          addr += 4;
          STM(addr, i);
          if (flag) {
            cpu.R[Rn] = addr - 4;
            flag = !flag;
          }
        }
      }
      STM_CLK
    } break;
    case 0x04: {
      uint32_t oldmode = cpu.CPSR.M;
      cpu_set_mode(MODE_USER);
      BT_PA
      addr -= bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          STM(addr, i);
        }
      cpu_set_mode(oldmode);
      STM_CLK
    } break;
    case 0x06: {
      uint32_t oldmode = cpu.CPSR.M;
      cpu_set_mode(MODE_USER);
      BT_PA
      addr -= bit << 2;
      bool flag = true;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          addr += 4;
          STM(addr, i);
          if (flag) {
            cpu.R[Rn] = addr - 4;
            flag = !flag;
          }
        }
      }
      cpu_set_mode(oldmode);
      STM_CLK
    } break;
    case 0x08: {
      BT_PA
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          STM(addr, i);
          addr += 4;
        }
      STM_CLK
    } break;
    case 0x0A: {
      BT_PA
      bool flag = true;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          STM(addr, i);
          addr += 4;
          if (flag) {
            cpu.R[Rn] = addr - 4 + (bit << 2);
            flag = !flag;
          }
        }
      }
      STM_CLK
    } break;
    case 0x0C: {
      uint32_t oldmode = cpu.CPSR.M;
      cpu_set_mode(MODE_USER);
      BT_PA
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          STM(addr, i);
          addr += 4;
        }
      cpu_set_mode(oldmode);
      STM_CLK
    } break;
    case 0x0E: {
      uint32_t oldmode = cpu.CPSR.M;
      cpu_set_mode(MODE_USER);
      BT_PA
      bool flag = true;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          STM(addr, i);
          addr += 4;
          if (flag) {
            cpu.R[Rn] = addr - 4 + (bit << 2);
            flag = !flag;
          }
        }
      }
      cpu_set_mode(oldmode);
      STM_CLK
    } break;
    case 0x10: {
      BT_PA
      addr -= bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          STM(addr, i);
          addr += 4;
        }
      STM_CLK
    } break;
    case 0x12: {
      BT_PA
      addr -= bit << 2;
      bool flag = true;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          STM(addr, i);
          addr += 4;
          if (flag) {
            cpu.R[Rn] = addr - 4;
            flag = !flag;
          }
        }
      }
      STM_CLK
    } break;
    case 0x14: {
      uint32_t oldmode = cpu.CPSR.M;
      cpu_set_mode(MODE_USER);
      BT_PA
      addr -= bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          STM(addr, i);
          addr += 4;
        }
      cpu_set_mode(oldmode);
      STM_CLK
    } break;
    case 0x16: {
      uint32_t oldmode = cpu.CPSR.M;
      cpu_set_mode(MODE_USER);
      BT_PA
      addr -= bit << 2;
      bool flag = true;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          STM(addr, i);
          addr += 4;
          if (flag) {
            cpu.R[Rn] = addr - 4;
            flag = !flag;
          }
        }
      }
      cpu_set_mode(oldmode);
      STM_CLK
    } break;
    case 0x18: {
      BT_PA
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          STM(addr, i);
        }
      STM_CLK
    } break;
    case 0x1A: {
      BT_PA
      bool flag = true;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          addr += 4;
          STM(addr, i);
          if (flag) {
            cpu.R[Rn] = addr - 4 + bit << 2;
            flag = !flag;
          }
        }
      }
      STM_CLK
    } break;
    case 0x1C: {
      uint32_t oldmode = cpu.CPSR.M;
      cpu_set_mode(MODE_USER);
      BT_PA
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          STM(addr, i);
        }
      cpu_set_mode(oldmode);
      STM_CLK
    } break;
    case 0x1E: {
      uint32_t oldmode = cpu.CPSR.M;
      cpu_set_mode(MODE_USER);
      BT_PA
      bool flag = true;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          addr += 4;
          STM(addr, i);
          if (flag) {
            cpu.R[Rn] = addr - 4 + (bit << 2);
            flag = !flag;
          }
        }
      }
      cpu_set_mode(oldmode);
      STM_CLK
    } break;
    case 0x01: {
      BT_PA
      addr -= bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          LDM(addr, i);
        }
      LDM_CLK
    } break;
    case 0x03: {
      BT_PA
      addr -= bit << 2;
      cpu.R[Rn] = addr;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          addr += 4;
          LDM(addr, i);
        }
      }
      LDM_CLK
    } break;
    case 0x05: {
      uint32_t oldmode = cpu.CPSR.M;
      if (BIT(opcode, 15))
        cpu.CPSR = cpu.SPSR;
      else
        cpu_set_mode(MODE_USER);
      BT_PA
      addr -= bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          LDM(addr, i);
        }
      LDM_CLK
      if (!(BIT(opcode, 15)))
        cpu_set_mode(oldmode);
    } break;
    case 0x07: {
      uint32_t oldmode = cpu.CPSR.M;
      if (BIT(opcode, 15))
        cpu.CPSR = cpu.SPSR;
      else
        cpu_set_mode(MODE_USER);
      BT_PA
      addr -= bit << 2;
      cpu.R[Rn] = addr;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          LDM(addr, i);
        }
      LDM_CLK
      if (!(BIT(opcode, 15)))
        cpu_set_mode(oldmode);
    } break;
    case 0x09: {
      BT_PA
      if (Rn == 15)
        addr += 8;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          LDM(addr, i);
          addr += 4;
        }
      LDM_CLK
    } break;
    case 0x0B: {
      BT_PA
      if (Rn == 15)
        addr += 8;
      else
        addr += bit << 2;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          LDM(addr, i);
          addr += 4;
        }
      }
      LDM_CLK
    } break;
    case 0x0D: {
      uint32_t oldmode = cpu.CPSR.M;
      if (BIT(opcode, 15))
        cpu.CPSR = cpu.SPSR;
      else
        cpu_set_mode(MODE_USER);
      BT_PA
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          LDM(addr, i);
          addr += 4;
        }
      LDM_CLK
      if (!(BIT(opcode, 15)))
        cpu_set_mode(oldmode);
    } break;
    case 0x0F: {
      uint32_t oldmode = cpu.CPSR.M;
      if (BIT(opcode, 15))
        cpu.CPSR = cpu.SPSR;
      else
        cpu_set_mode(MODE_USER);
      BT_PA
      cpu.R[Rn] += bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          LDM(addr, i);
          addr += 4;
        }
      LDM_CLK
      if (!(BIT(opcode, 15)))
        cpu_set_mode(oldmode);
    } break;
    case 0x11: {
      BT_PA
      addr -= bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          LDM(addr, i);
          addr += 4;
        }
      LDM_CLK
    } break;
    case 0x13: {
      BT_PA
      addr -= bit << 2;
      cpu.R[Rn] = addr;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          LDM(addr, i);
          addr += 4;
        }
      }
      LDM_CLK
    } break;
    case 0x15: {
      uint32_t oldmode = cpu.CPSR.M;
      if (BIT(opcode, 15))
        cpu.CPSR = cpu.SPSR;
      else
        cpu_set_mode(MODE_USER);
      BT_PA
      addr -= bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          LDM(addr, i);
          addr += 4;
        }
      LDM_CLK
      if (!(BIT(opcode, 15)))
        cpu_set_mode(oldmode);
    } break;
    case 0x17: {
      uint32_t oldmode = cpu.CPSR.M;
      if (BIT(opcode, 15))
        cpu.CPSR = cpu.SPSR;
      else
        cpu_set_mode(MODE_USER);
      BT_PA
      addr -= bit << 2;
      cpu.R[Rn] = addr;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          LDM(addr, i);
          addr += 4;
        }
      LDM_CLK
      if (!(BIT(opcode, 15)))
        cpu_set_mode(oldmode);
    } break;
    case 0x19: {
      BT_PA
      if (Rn == 15)
        addr += 8;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          LDM(addr, i);
        }
      LDM_CLK
    } break;
    case 0x1B: {
      BT_PA
      if (Rn == 15)
        addr += 8;
      else
        addr += bit << 2;
      for (int i = 0; i < 16; i++) {
        if (BIT(opcode, i)) {
          addr += 4;
          LDM(addr, i);
        }
      }
      LDM_CLK
    } break;
    case 0x1D: {
      uint32_t oldmode = cpu.CPSR.M;
      if (BIT(opcode, 15))
        cpu.CPSR = cpu.SPSR;
      else
        cpu_set_mode(MODE_USER);
      BT_PA
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          LDM(addr, i);
        }
      LDM_CLK
      if (!(BIT(opcode, 15)))
        cpu_set_mode(oldmode);
    } break;
    case 0x1F: {
      uint32_t oldmode = cpu.CPSR.M;
      if (BIT(opcode, 15))
        cpu.CPSR = cpu.SPSR;
      else
        cpu_set_mode(MODE_USER);
      BT_PA
      cpu.R[Rn] += bit << 2;
      for (int i = 0; i < 16; i++)
        if (BIT(opcode, i)) {
          addr += 4;
          LDM(addr, i);
        }
      LDM_CLK
      if (!(BIT(opcode, 15)))
        cpu_set_mode(oldmode);
    } break;
    default:
      // Fuck CLion.
      break;
  }
}

DECL(single_swap) {
  uint32_t val = cpu.R[opcode & 0xF];
  uint32_t addr = cpu.R[(opcode >> 16) & 0xF];
  uint32_t dist_reg = (opcode >> 12) & 0xF;
  if (BIT(opcode, 22)) {
    cpu.R[dist_reg] = memory_read_8(addr);
    memory_write_8(addr, (uint8_t)val);
  } else {
    cpu.R[dist_reg] = memory_read_32(addr);
    memory_write_32(addr, val);
  }

  clocks -= get_access_cycles(isSeq, 1, cpu.R[R_PC]) +
            (get_access_cycles_nonseq32(addr) * 2) + 1;
}

DECL(swi) {
  cpu_set_mode(MODE_SUPERVISOR);
  cpu.R[R_LR] = cpu.R[R_PC] + 4;
  cpu.SPSR = cpu.CPSR;
  cpu.CPSR.I = 1;
  cpu.CPSR.T = 0;
  cpu.R[R_PC] = 4;
  clocks -= get_access_cycles_seq32(cpu.R[R_PC]) +
            get_access_cycles_nonseq32(cpu.R[R_PC]) +
            get_access_cycles(isSeq, 1, cpu.PC_old);
}

DECL(ill) {
  log_warn("Illegal opcode: 0x%8x", opcode);
  clocks -= 100;  // Away from trap
}
static arm_block arm_code[] = {
    arm_branch_and_exchange,
    arm_branch_with_link,
    arm_data_processing,
    arm_psr,
    arm_multiply,
    arm_single_transfer,
    arm_half_transfer,
    arm_block_transfer,
    arm_single_swap,
    arm_swi,
    arm_ill,
};

bool cond(opcode) {
  switch ((opcode >> 28) & 0xF) {
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
    case 0xE:
      return COND(AL);
    default:
      return false;
  }
}

int8_t cpu_decode_arm(int32_t opcode) {
  if ((opcode & 0x0FFFFF00) == 0x012FFF00) {
    return 0;  // branch and exchange
  } else if ((opcode & 0x0E000000) == 0x0A000000) {
    return 1;  // branch with link
  } else if ((opcode & 0xD900000) == 0x1000000) {
    if (BIT(opcode, 7) && (BIT(opcode, 4) && !((BIT(opcode, 25))))) {
      if (opcode & 0x00000060)
        return 8;  // single data swap
      else
        return 6;  // halfword, doubleword, and signed data transfer
    } else
      return 3;  // psr instructions
  } else if ((opcode & 0x0C000000) == 0) {
    if (BIT(opcode, 7) && !(BIT(opcode, 4))) {
      if ((opcode & 0x02000000) ||
          ((opcode & 0x00100000) && (opcode & 0x01800000) == 0x01000000) ||
          (opcode & 0x01800000) != 0x01000000)
        return 2;  // data processing
      else
        return 4;  // multiply
    } else if (BIT(opcode, 7) && BIT(opcode, 4)) {
      if ((opcode & 0x000000F0) == 0x00000090) {
        if ((opcode & 0x02000000))
          return 2;  // data processing
        else if ((opcode & 0x01800000) != 0x01000000)
          return 8;  // single data swap
        else
          return 4;  // multiply
      } else if ((opcode & 0x02000000))
        return 2;  // data processing
      else
        return 6;  // halfword, doubleword, and signed data transfer
    } else
      return 2;  // data processing
  } else if ((opcode & 0x0C000000) == 0x04000000)
    return 5;  // single data transfer
  else if ((opcode & 0x0E000000) == 0x08000000)
    return 7;  // block data transfer
  else if ((opcode & 0x0F000000) == 0x0F000000)
    return 9;  // swi
  else
    return 10;  // illegal instruction block
}

void cpu_run_arm(int64_t clock) {
  clocks = clock;
  while (clocks > 0) {
    isSeq = (cpu.PC_old + 4 == cpu.R[R_PC]);
    cpu.PC_old = cpu.R[R_PC];
    register uint32_t opcode = memory_read_32(cpu.R[R_PC]);
    if (cond(opcode))
      arm_code[cpu_decode_arm(opcode)](opcode);
    else
      clocks -= get_access_cycles(isSeq, 1, cpu.R[R_PC]);
    cpu.R[R_PC] += 4;
  }
}
