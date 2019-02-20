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
#include <log.h>
#include <stdint.h>
#define F_N (cpu.CPSR.N)
#define F_Z (cpu.CPSR.Z)
#define F_C (cpu.CPSR.C)
#define F_V (cpu.CPSR.V)

#define F_T (cpu.CPSR.T)  // 0 as ARM, 1 as THUMB

#define COND(TYPE) (COND_##TYPE != 0)
#define COND_EQ (F_Z)
#define COND_NE (!F_Z)
#define COND_CS (F_C)
#define COND_CC (!F_C)
#define COND_MI (F_N)
#define COND_PL (!F_N)
#define COND_VS (F_V)
#define COND_VC (!F_V)
#define COND_HI ((F_C) && (!F_Z))
#define COND_LS ((!F_C) || (F_Z))
#define COND_GE ((!F_N) == (!F_V))
#define COND_LT ((!F_N) != (!F_V))
#define COND_GT ((!F_Z) && (!F_N) == (!F_V))
#define COND_LE ((F_Z) || (!F_N) != (!F_V))
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

#define SHIFT_BY(M, CARRY)                            \
  static inline uint32_t cpu_shift_by_##M##CARRY(     \
      uint32_t type, uint32_t value, uint8_t shift) { \
    switch (type & 0x3) {                             \
      case 0:                                         \
        return lsl_by_##M##CARRY(value, shift);       \
      case 1:                                         \
        return lsr_by_##M##CARRY(value, shift);       \
      case 2:                                         \
        return asr_by_##M##CARRY(value, shift);       \
      case 3:                                         \
        return ror_by_##M##CARRY(value, shift);       \
      default:                                        \
        return 0;                                     \
    }                                                 \
    return 0;                                         \
  }

SHIFT_BY(imm, )
SHIFT_BY(imm, _carry)
SHIFT_BY(reg, )
SHIFT_BY(reg, _carry)
#endif
