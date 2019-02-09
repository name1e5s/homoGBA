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
#ifndef MACROS_H
#define MACROS_H
#define _K *1024
#define BIT(VAL, N) ((VAL >> N) & 0x1)
#define REP_2(BODY) BODY, BODY
#define REP_4(BODY) BODY, BODY, BODY, BODY
#define REP_8(BODY) REP_2(REP_4(BODY))
#define REP_16(BODY) REP_4(REP_4(BODY))
#define REP_64(BODY) REP_4(REP_4(REP_4(BODY)))
#define REP_256(BODY) REP_4(REP_4(REP_4(REP_4(BODY))))
#endif
