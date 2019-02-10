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