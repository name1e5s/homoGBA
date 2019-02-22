#include <stdio.h>

int main(void) {
  disas_arm(0xE1A01001);
  disas_thumb(0x4770);
  return 1;
}
