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
#ifndef CPU_H
#define CPU_H
#include <macros.h>
#include <stdint.h>
typedef enum {
  MODE_USER = 0x10,
  MODE_FIQ = 0x11,
  MODE_IRQ = 0x12,
  MODE_SUPERVISOR = 0x13,
  MODE_ABORT = 0x14,
  MODE_UNDEFINED = 0x15,
  MODE_SYSTEM = 0x16,
} cpu_mode;

typedef enum {
  EXEC_ARM,
  EXEC_THUMB,
} cpu_exec_mode;

#define DECL_REG(MODE) \
  uint32_t R13_##MODE; \
  uint32_t R14_##MODE; \
  cpsr SPSR_##MODE;

#define R_SP (13)
#define R_LR (14)
#define R_PC (15)

typedef struct cpsr {
  unsigned N : 1;
  unsigned Z : 1;
  unsigned C : 1;
  unsigned V : 1;
  unsigned Q : 1;
  unsigned I : 1;
  unsigned F : 1;
  unsigned T : 1;
  unsigned M : 5;
} cpsr;

static inline uint32_t cpsr_to_uint(cpsr reg) {
  return (uint32_t)((reg.N << 31) | (reg.Z << 30) | (reg.C << 29) |
                    (reg.V << 28) | (reg.Q << 27) | (reg.I << 7) |
                    (reg.F << 6) | (reg.T << 5) | (reg.M));
}

static inline cpsr uint_to_cpsr(uint32_t data) {
  cpsr reg = {
      .N = BIT(data, 31),
      .Z = BIT(data, 30),
      .C = BIT(data, 29),
      .V = BIT(data, 28),
      .Q = BIT(data, 27),
      .I = BIT(data, 7),
      .F = BIT(data, 6),
      .T = BIT(data, 5),
      .M = data & 0xF,
  };
  return reg;
}

typedef struct homo_cpu {
  cpu_exec_mode exec_mode;
  cpu_mode mode;

  uint32_t R[16];
  cpsr CPSR;
  cpsr SPSR;

  uint32_t PC_old;

  uint32_t R_user[7];

  uint32_t R_fiq[7];
  cpsr SPSR_fiq;

  DECL_REG(svc)
  DECL_REG(abt)
  DECL_REG(irq)
  DECL_REG(und)
} cpu_t;

extern cpu_t cpu;

// TODO: Not a good practice.
extern int64_t clocks;

void cpu_init(void);
void cpu_set_mode(uint32_t mode);
void cpu_run(int64_t clock);
extern void cpu_run_arm();
extern void cpu_run_thumb();
#endif
