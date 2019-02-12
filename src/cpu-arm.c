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

int64_t cpu_run_arm(int64_t clocks) {
  while (clocks > 0) {
    bool isSeq = (cpu.PC_old + 4 == cpu.R[R_PC]);
    cpu.PC_old = cpu.R[R_PC];

    uint32_t opcode = memory_read_32(cpu.R[R_PC]);

    uint8_t kase = (uint8_t)((opcode >> 0x18) & (BIT(opcode, 5)));
    switch (kase) {
      default:
        log_warn("cpu: Unknown instruction 0x%x.", opcode);
        break;
    }
    cpu.R[R_PC] += 4;
  }
  return clocks;
}