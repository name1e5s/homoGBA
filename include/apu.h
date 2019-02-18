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
#ifndef APU_H
#define APU_H
#include <stdbool.h>
#include <stdint.h>

void apu_init(void);
bool apu_sound_is_on(void);
int32_t apu_update(int32_t clocks);
void apu_register_write16(uint32_t address, uint16_t value);
void apu_reset_buffer(void);
void apu_sound_end(void);
void apu_callback(void* buffer, size_t length);
void apu_timer_check(int num);
#endif
