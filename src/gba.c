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
#include <cartridge.h>
#include <cpu.h>
#include <gba.h>
#include <stdlib.h>

int gba_init(const char* path) {
  uint8_t* gamepak = cartridge_load(path);
  memory.rom_wait0 = gamepak;
  memory.rom_wait1 = gamepak;
  memory.rom_wait2 = gamepak;

  cpu_init();
}

void gba_delete() {
  free(memory.rom_wait0);
}