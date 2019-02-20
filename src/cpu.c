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

cpu_t cpu;
int64_t clocks;

void cpu_init(void) {
  cpu.exec_mode = EXEC_ARM;
  /*
  cpu.R[R_SP] = 0x03007F00;
  cpu.R[R_LR] = 0x08000000;
  cpu.R[R_PC] = 0x00000000;
*/
  cpu.CPSR.M = MODE_SUPERVISOR;
  cpu.CPSR.F = 1;
  cpu.CPSR.I = 1;

  cpu.R_user[5] = 0x03007F00;
  cpu.R13_svc = 0x03007FE0;
  cpu.R13_irq = 0x03007FA0;

  cpu.PC_old = 0;
}

#define SAVE_REG(MODE)            \
  for (int i = 0; i < 5; i++)     \
    cpu.R_user[i] = cpu.R[i + 8]; \
  cpu.R13_##MODE = cpu.R[13];     \
  cpu.R14_##MODE = cpu.R[14];     \
  cpu.SPSR_##MODE = cpu.SPSR;

#define LOAD_REG(MODE)            \
  for (int i = 0; i < 5; i++)     \
    cpu.R[i + 8] = cpu.R_user[i]; \
  cpu.R[13] = cpu.R13_##MODE;     \
  cpu.R[14] = cpu.R14_##MODE;     \
  cpu.SPSR = cpu.SPSR_##MODE;

void cpu_set_mode(uint32_t mode) {
  if (cpu.CPSR.M == mode)
    return;

  // Save Registers
  if (cpu.CPSR.M == MODE_USER || cpu.CPSR.M == MODE_SYSTEM)
    for (int i = 0; i < 7; i++)
      cpu.R_user[i] = cpu.R[i + 8];

  if (cpu.CPSR.M == MODE_FIQ) {
    for (int i = 0; i < 7; i++)
      cpu.R_fiq[i] = cpu.R[i + 8];
    cpu.SPSR_fiq = cpu.SPSR;
  }

  if (cpu.CPSR.M == MODE_IRQ) {
    SAVE_REG(irq)
  }
  if (cpu.CPSR.M == MODE_SUPERVISOR) {
    SAVE_REG(svc)
  }

  if (cpu.CPSR.M == MODE_ABORT) {
    SAVE_REG(abt)
  }

  if (cpu.CPSR.M == MODE_UNDEFINED) {
    SAVE_REG(und)
  }

  cpu.CPSR.M = mode;

  // Load Registers
  if (cpu.CPSR.M == MODE_USER || cpu.CPSR.M == MODE_SYSTEM)
    for (int i = 0; i < 7; i++)
      cpu.R[i + 8] = cpu.R_user[i];

  if (cpu.CPSR.M == MODE_FIQ) {
    for (int i = 0; i < 7; i++)
      cpu.R[i + 8] = cpu.R_fiq[i];
    cpu.SPSR_fiq = cpu.SPSR;
  }

  if (cpu.CPSR.M == MODE_IRQ) {
    LOAD_REG(irq)
  }
  if (cpu.CPSR.M == MODE_SUPERVISOR) {
    LOAD_REG(svc)
  }

  if (cpu.CPSR.M == MODE_ABORT) {
    LOAD_REG(abt)
  }

  if (cpu.CPSR.M == MODE_UNDEFINED) {
    LOAD_REG(und)
  }
}

void cpu_run(int64_t clock) {
  clocks = clock;
  if (cpu.exec_mode == EXEC_ARM)
    cpu_run_arm();
  else
    cpu_run_thumb();
}