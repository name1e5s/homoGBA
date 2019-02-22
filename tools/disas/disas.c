#include <stdio.h>

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <bfd.h>

#include <dis-asm.h>

int disas_fprintf(void* stream, const char* format, ...) {
  va_list arg;

  va_start(arg, format);
  vfprintf(stdout, format, arg);
  return 0;
}

void disas_arm(uint32_t opcode) {
  unsigned int data[1] = {opcode};
  bfd_byte* buf = (bfd_byte*)&data[0];
  disassemble_info* c =
      (struct disassemble_info*)calloc(1, sizeof(disassemble_info));
  init_disassemble_info(c, stdout, disas_fprintf);
  c->arch = bfd_arch_arm;
  c->mach = bfd_mach_arm_unknown;
  disassemble_init_for_target(c);
  c->buffer_vma = 0;
  c->buffer = buf;
  c->buffer_length = 4;

  unsigned int count = 0;
  size_t pos = 0;
  size_t length = c->buffer_length;
  size_t max_pos = c->buffer_vma + length;

  while (pos < max_pos) {
    unsigned int size = print_insn_little_arm((bfd_vma)pos, c);
    pos += size;
    count++;
    fprintf(stdout, "\n");
  }
}

void disas_thumb(uint16_t opcode) {
  uint16_t data[1] = {opcode};
  bfd_byte* buf = (bfd_byte*)&data[0];
  disassemble_info* c =
      (struct disassemble_info*)calloc(1, sizeof(disassemble_info));
  init_disassemble_info(c, stdout, disas_fprintf);
  c->arch = bfd_arch_arm;
  c->mach = bfd_mach_arm_unknown;
  c->disassembler_options = "force-thumb";
  disassemble_init_for_target(c);
  c->buffer_vma = 0;
  c->buffer = buf;
  c->buffer_length = 2;

  unsigned int count = 0;
  size_t pos = 0;
  size_t length = c->buffer_length;
  size_t max_pos = c->buffer_vma + length;

  while (pos < max_pos) {
    unsigned int size = print_insn_little_arm((bfd_vma)pos, c);
    pos += size;
    count++;
    fprintf(stdout, "\n");
  }
}

