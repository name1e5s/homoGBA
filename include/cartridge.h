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
#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <stdint.h>

typedef struct homo_cartridge_header {
  uint32_t entry_point;
  uint8_t logo[156];
  char title[12];
  char game_code[4];
  char maker_code[2];
  uint8_t fixed_value;
  uint8_t unit;
  uint8_t device;
  uint8_t reversed[7];
  uint8_t version;
  uint8_t checksum;
  uint8_t zero[2];
  // For multiboot
  uint32_t ram_entry_point;
  uint8_t boot_mode;
  uint8_t slave_number;
  uint8_t unused[26];
  uint32_t joybus_entry;
} cartridge_header;

uint8_t* cartridge_load(const char* path);
#endif
