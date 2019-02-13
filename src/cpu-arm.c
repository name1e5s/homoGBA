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

#define DECL(INSTR) static inline void arm_##INSTR(uint32_t opcode)

#define CALL(INSTR) arm_##INSTR(opcode)

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
      ALU_3OP(and##S_##TYPE);     \
      break;                      \
    case 0x1:                     \
      ALU_3OP(eor##S_##TYPE);     \
      break;                      \
    case 0x2:                     \
      ALU_3OP(sub##S_##TYPE);     \
      break;                      \
    case 0x3:                     \
      ALU_3OP(rsb##S_##TYPE);     \
      break;                      \
    case 0x4:                     \
      ALU_3OP(add##S_##TYPE);     \
      break;                      \
    case 0x5:                     \
      ALU_3OP(adc##S_##TYPE);     \
      break;                      \
    case 0x6:                     \
      ALU_3OP(sbc##S_##TYPE);     \
      break;                      \
    case 0x7:                     \
      ALU_3OP(rsc##S_##TYPE);     \
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
      ALU_3OP(orr##S_##TYPE);     \
      break;                      \
    case 0xD:                     \
      ALU_3OP(mov##S_##TYPE);     \
      break;                      \
    case 0xE:                     \
      ALU_3OP(bic##S_##TYPE);     \
      break;                      \
    case 0xF:                     \
      ALU_3OP(mvn##S_##TYPE);     \
      break;                      \
    default:                      \
      break;                      \
  }

int64_t cpu_run_arm(int64_t clocks) {
  while (clocks > 0) {
    bool isSeq = (cpu.PC_old + 4 == cpu.R[R_PC]);
    cpu.PC_old = cpu.R[R_PC];

    register uint32_t opcode = memory_read_32(cpu.R[R_PC]);
    // Inspired by GNU libopcodes.
    /* --- BEGIN DECODING --- */
    if ((opcode & 0xfe000000) ==
        0xfa000000) {  // Branch and Branch with Link (B, BL, BLX_imm)
      uint32_t nn = opcode & 0x00FFFFFF;
      if (BIT(nn, 23))
        nn |= 0xFF000000;
      if (BIT(opcode, 24))
        cpu.R[R_LR] = cpu.R[R_PC] + 4;
      cpu.R[R_PC] += 8 + (nn << 2) - 4;
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) +
                get_access_cycles_seq32(cpu.R[R_PC]) +
                get_access_cycles_nonseq32(cpu.R[R_PC]);
    } else if ((opcode & 0x0fffff00) ==
               0x012fff00) {  // Branch and Exchange (BX, BLX_reg)
      uint32_t Rn = opcode & 0xF;
      if (BIT(opcode, 5))
        cpu.R[R_LR] = cpu.R[R_PC] + 4;

      if (cpu.R[Rn] & 0x1) {
        cpu.exec_mode = EXEC_THUMB;
        cpu.CPSR.T = 1;
        cpu.R[R_PC] = (cpu.R[Rn] + (Rn == R_PC ? 8 : 0)) & (~1);
        clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) * 2 +
                  get_access_cycles_nonseq32(cpu.R[R_PC]);
        return cpu_run_thumb(clocks);
      }

      cpu.R[R_PC] = (cpu.R[Rn] + (Rn == R_PC ? 8 : 0)) & (~3) - 4;
      clocks -= get_access_cycles(isSeq, 1, cpu.PC_old) * 2 +
                get_access_cycles_nonseq32(cpu.R[R_PC]);
    } // TODO: PSR
    /* ---  END DECODING  --- */
    cpu.R[R_PC] += 4;
  }
  return clocks;
}
