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
  uint32_t SPSR_##MODE;

#define R_SP (13)
#define R_LR (14)
#define R_PC (15)

typedef struct homo_cpu {
  cpu_exec_mode exec_mode;
  cpu_mode mode;

  uint32_t R[16];
  uint32_t CPSR;
  uint32_t SPSR;

  uint32_t PC_old;

  uint32_t R_user[7];

  uint32_t R_fiq[7];
  uint32_t SPSR_fiq;

  DECL_REG(svc)
  DECL_REG(abt)
  DECL_REG(irq)
  DECL_REG(und)
} cpu_t;

extern cpu_t cpu;

#define F_N (BIT(cpu.CPSR, 31))
#define F_Z (BIT(cpu.CPSR, 30))
#define F_C (BIT(cpu.CPSR, 29))
#define F_V (BIT(cpu.CPSR, 28))

#define F_EXEC BIT(cpu.CPSR, 5)  // 0 as ARM, 1 as THUMB

// Conditions
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

#define CARRY_ADD(M, N, D) \
  (((uint32_t)(M) >> 31) + ((uint32_t)(N) >> 31) > ((uint32_t)(D) >> 31))
#define BORROW_SUB(M, N, D) (((uint32_t)(M)) >= ((uint32_t)(N)))
#define BORROW_SUB_CARRY(M, N, D, C) (UXT_64(M) >= (UXT_64(N)) + (uint64_t)(C))
#define V_ADD(M, N, D) \
  (!(SIGN((M) ^ (N))) && (SIGN((M) ^ (D))) && (SIGN((N) ^ (D))))
#define V_SUB(M, N, D) ((SIGN((M) ^ (N))) && (SIGN((M) ^ (D))))

void init_cpu(void);
#endif
