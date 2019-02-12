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
#ifndef CPU_ARM_H
#define CPU_ARM_H
// Conditions
#include <cpu.h>
#include <stdint.h>
#define F_N (cpu.CPSR.N)
#define F_Z (cpu.CPSR.Z)
#define F_C (cpu.CPSR.C)
#define F_V (cpu.CPSR.V)

#define F_T (cpu.CPSR.T)  // 0 as ARM, 1 as THUMB

#define COND_EQ (F_Z)
#define COND_NE (!F_Z)
#define COND_CS (F_C)
#define COND_CC (!F_C)
#define COND_MI (F_N)
#define COND_PL (!F_N)
#define COND_VS (F_V)
#define COND_VC (!F_V)
#define COND_HI (F_C && !F_Z)
#define COND_LS (!F_C || F_Z)
#define COND_GE (!F_N == !F_V)
#define COND_LT (!F_N != !F_V)
#define COND_GT (!F_Z && !F_N == !F_V)
#define COND_LE (F_Z || !F_N != !F_V)
#define COND_AL 1

#define SIGN(N) ((N) >> 31)

// Flags
#define CARRY_ADD(M, N, D) \
  (((uint32_t)(M) >> 31) + ((uint32_t)(N) >> 31) > ((uint32_t)(D) >> 31))
#define BORROW_SUB(M, N, D) (((uint32_t)(M)) >= ((uint32_t)(N)))
#define BORROW_SUB_CARRY(M, N, D, C) (UXT_64(M) >= (UXT_64(N)) + (uint64_t)(C))
#define V_ADD(M, N, D) \
  (!(SIGN((M) ^ (N))) && (SIGN((M) ^ (D))) && (SIGN((N) ^ (D))))
#define V_SUB(M, N, D) ((SIGN((M) ^ (N))) && (SIGN((M) ^ (D))))

// Shift Operations

// Build the immediate value.
static inline uint32_t build_imm(uint32_t value, uint8_t shift) {
  return (value >> shift) | (value << (32 - shift));
}

static inline uint32_t build_imm_carry(uint32_t value, uint8_t shift) {
  if (shift != 0) {
    F_C = (value & (ONE(shift - 1)));
    return (value >> shift) | (value << (32 - shift));
  }
  return value;
}

// Shift by immediate value.
#define BY_IMM(TYPE, BODY, BODY_AT_ZERO, CARRY, CARRY_AT_ZERO)                \
  static inline uint32_t TYPE##_by_imm(uint32_t value, uint8_t shift) {       \
    if (shift != 0)                                                           \
      return (BODY);                                                          \
    return (BODY_AT_ZERO);                                                    \
  }                                                                           \
  static inline uint32_t TYPE##_by_imm_carry(uint32_t value, uint8_t shift) { \
    if (shift != 0) {                                                         \
      F_C = (CARRY);                                                          \
      return (BODY);                                                          \
    }                                                                         \
    F_C = (CARRY_AT_ZERO);                                                    \
    return (BODY_AT_ZERO);                                                    \
  }

BY_IMM(lsl, value << shift, value, BIT(value, 32 - shift), cpu.CPSR.C)
BY_IMM(lsr, value >> shift, 0, BIT(value, shift - 1), BIT(value, 31))
BY_IMM(asr,
       (uint32_t)(((int32_t)value) >> shift),
       F_C ? 0xFFFFFFFF : 0,
       BIT(value, shift - 1),
       BIT(value, 31))
BY_IMM(ror,
       (value >> shift) | (value << (32 - shift)),
       (value >> 1) | (F_C ? ONE(31) : 0),
       BIT(value, shift - 1),
       BIT(value, 0))

// Shift by registers.
#define BY_REG(TYPE, BODY, BODY_EQUAL_TO_32, BODY_LARGER_THAN_32,             \
               BODY_AT_ZERO, CARRY, CARRY_EQUAL_TO_32, CARRY_LARGER_THAN_32,  \
               CARRY_AT_ZERO)                                                 \
  static inline uint32_t TYPE##_by_reg(uint32_t value, uint8_t shift) {       \
    if (shift != 0) {                                                         \
      if (shift < 32)                                                         \
        return (BODY);                                                        \
      else if (shift == 32)                                                   \
        return (BODY_EQUAL_TO_32);                                            \
      else                                                                    \
        return (BODY_LARGER_THAN_32);                                         \
    }                                                                         \
    return (BODY_AT_ZERO);                                                    \
  }                                                                           \
  static inline uint32_t TYPE##_by_reg_carry(uint32_t value, uint8_t shift) { \
    if (shift != 0) {                                                         \
      if (shift < 32) {                                                       \
        F_C = (CARRY);                                                        \
        return (BODY);                                                        \
      } else if (shift == 32) {                                               \
        F_C = (CARRY_EQUAL_TO_32);                                            \
        return (BODY_EQUAL_TO_32);                                            \
      } else {                                                                \
        F_C = (CARRY_LARGER_THAN_32);                                         \
        return (BODY_LARGER_THAN_32);                                         \
      }                                                                       \
    }                                                                         \
    F_C = (CARRY_AT_ZERO);                                                    \
    return (BODY_AT_ZERO);                                                    \
  }

BY_REG(lsl,
       value << shift,
       0,
       0,
       value,
       BIT(value, 32 - shift),
       BIT(value, 0),
       0,
       cpu.CPSR.C)
BY_REG(lsr,
       value >> shift,
       0,
       0,
       0,
       BIT(value, shift - 1),
       BIT(value, 0),
       0,
       BIT(value, 31))
BY_REG(asr,
       (uint32_t)(((int32_t)value) >> shift),
       (uint32_t)(((int32_t)value) >> 31),
       (uint32_t)(((int32_t)value) >> 31),
       value,
       BIT(value, shift - 1),
       BIT(value, 31),
       BIT(value, 31),
       F_C)

static inline uint32_t ror_by_reg(uint32_t value, uint8_t shift) {
  if (shift != 0) {
    shift = shift & 0x1F;
    if (shift != 0)
      return (value >> shift) | (value << (32 - shift));
    else
      return value;
  } else
    return value;
}

static inline uint32_t ror_by_reg_carry(uint32_t value, uint8_t shift) {
  if (shift != 0) {
    shift = shift & 0x1F;
    if (shift != 0) {
      F_C = BIT(value, shift - 1);
      return (value >> shift) | (value << (32 - shift));
    } else {
      F_C = BIT(value, 31);
      return value;
    }
  } else
    return value;
}

// ALU Instructions.
#define ALU_3OP_IMM(INSN, N, Z, C, V, CARRY, PRE_BODY, BODY) \
  static inline void arm_##INSN##_imm(uint32_t opcode) {     \
    uint32_t Rd = (opcode >> 12) & 0xF;                      \
    uint32_t Rn = (opcode >> 16) & 0xF;                      \
    uint32_t val = opcode & 0xFF;                            \
    uint32_t ror = (opcode >> 7) & 0x1E;                     \
    PRE_BODY                                                 \
    uint32_t Rn_val = cpu.R[Rn];                             \
    if (Rn == R_PC)                                          \
      Rn_val += 8;                                           \
    uint32_t imm = build_imm(val, ror);                      \
    BODY if (Rd == R_PC) cpu.R[Rd] -= 4;                     \
  }                                                          \
  static inline void arm_##INSN##s_imm(uint32_t opcode) {    \
    uint32_t Rd = (opcode >> 12) & 0xF;                      \
    uint32_t Rn = (opcode >> 16) & 0xF;                      \
    uint32_t val = opcode & 0xFF;                            \
    uint32_t ror = (opcode >> 7) & 0x1E;                     \
    PRE_BODY                                                 \
    uint32_t Rn_val = cpu.R[Rn];                             \
    if (Rn == R_PC)                                          \
      Rn_val += 8;                                           \
    uint32_t imm = build_imm##CARRY(val, ror);               \
    BODY F_N = (N);                                          \
    F_Z = (Z);                                               \
    F_C = (C);                                               \
    F_V = (V);                                               \
    if (Rd == R_PC) {                                        \
      if (cpu.CPSR.M != MODE_USER) {                         \
        cpu.CPSR = cpu.SPSR;                                 \
        cpu_set_mode(cpu.SPSR.M);                            \
      }                                                      \
      cpu.R[Rd] -= 4;                                        \
    }                                                        \
  }

ALU_3OP_IMM(and,
            BIT(cpu.R[Rd], 31),
            cpu.R[Rd] == 0,
            F_C,
            F_V,
            _carry,
            ,
            cpu.R[Rd] = Rn_val & imm;)
ALU_3OP_IMM(eor,
            BIT(cpu.R[Rd], 31),
            cpu.R[Rd] == 0,
            F_C,
            F_V,
            _carry,
            ,
            cpu.R[Rd] = Rn_val ^ imm;)
ALU_3OP_IMM(sub,
            BIT(cpu.R[Rd], 31),
            cpu.R[Rd] == 0,
            (tmp >> 32) != 0,
            V_ADD(Rn_val, (uint32_t)(uint64_t)(~imm), cpu.R[Rd]) != 0,
            ,
            ,
            uint64_t tmp = (uint64_t)Rn_val + (uint64_t)(~imm) + 1ULL;
            cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_IMM(rsb,
            BIT(cpu.R[Rd], 31),
            cpu.R[Rd] == 0,
            (tmp >> 32) != 0,
            V_ADD(imm, (uint32_t)(uint64_t)(~Rn_val), cpu.R[Rd]) != 0,
            ,
            ,
            uint64_t tmp = (uint64_t)(~Rn_val) + (uint64_t)(imm) + 1ULL;
            cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_IMM(add,
            BIT(cpu.R[Rd], 31),
            cpu.R[Rd] == 0,
            (tmp >> 32) != 0,
            V_ADD(Rn_val, imm, cpu.R[Rd]) != 0,
            ,
            ,
            uint64_t tmp = (uint64_t)(Rn_val) + (uint64_t)(imm);
            cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_IMM(adc,
            BIT(cpu.R[Rd], 31),
            cpu.R[Rd] == 0,
            (tmp >> 32) != 0,
            V_ADD(Rn_val, imm, cpu.R[Rd]) != 0,
            ,
            uint64_t prev_c = F_C;
            , uint64_t tmp = (uint64_t)(Rn_val) + (uint64_t)(imm) + prev_c;
            cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_IMM(sbc,
            BIT(cpu.R[Rd], 31),
            cpu.R[Rd] == 0,
            (tmp >> 32) != 0,
            V_ADD(Rn_val, (uint32_t)(uint64_t)(~imm), (uint32_t)tmp) != 0,
            ,
            uint64_t prev_c = F_C;
            ,
            uint64_t tmp = (uint64_t)Rn_val + (uint64_t)(~imm) + 1ULL - prev_c;
            cpu.R[Rd] = (uint32_t)tmp;)
ALU_3OP_IMM(rsc,
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
ALU_3OP_IMM(orr,
            BIT(cpu.R[Rd], 31),
            cpu.R[Rd] == 0,
            F_C,
            F_V,
            _carry,
            ,
            cpu.R[Rd] = Rn_val | imm;)
ALU_3OP_IMM(bic,
            BIT(cpu.R[Rd], 31),
            cpu.R[Rd] == 0,
            F_C,
            F_V,
            _carry,
            ,
            cpu.R[Rd] = Rn_val & (~imm);)
            
#endif