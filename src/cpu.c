#include <cpu.h>

cpu_t cpu;

void init_cpu(void) {
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