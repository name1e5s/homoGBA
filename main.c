#include <gba.h>
#include <log.h>
#include <stdio.h>

int main(int argc, char** argv) {
  init_gba("../test/SUPERMAR.GBA");
  dele_gba();
  return 0;
}
