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
#include <ppu.h>
void ppu_init(void) {}
void ppu_update_draw_scanline(void) {}
void ppu_draw_scanline(int32_t y) {}
void ppu_draw_scanline_white(int32_t y) {}
void ppu_update_register(uint32_t address) {}

void util_buffer_to_RGB_24(void* dest) {}
void util_buffer_to_RGB_32(void* dest) {}