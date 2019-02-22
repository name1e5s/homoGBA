#ifndef HOMO_DISAS
#define HOMO_DISAS
#include <stdint.h>

void disas_thumb(uint16_t opcode);
void disas_arm(uint32_t opcode);
#endif
