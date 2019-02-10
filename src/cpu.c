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

void cpu_init(void) {
  cpu.exec_mode = EXEC_ARM;
  cpu.R[R_SP] = 0x03007F00;
  cpu.R[R_LR] = 0x08000000;
  cpu.R[R_PC] = 0x08000000;

  cpu.CPSR = MODE_SUPERVISOR | (1 << 7) | (1 << 6);

  cpu.R_user[5] = 0x03007F00;
  cpu.R13_svc = 0x03007FE0;
  cpu.R13_irq = 0x03007FA0;

  cpu.PC_old = 0;
}
