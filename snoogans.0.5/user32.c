/*
 * Copyright (C) 2010 gonzoj
 *
 * Please check the CREDITS file for further information.
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

#include <stdlib.h>

#include "user32.h"

#define FUNC(r, a, f, p) typedef r a (*f##_t)p; f##_t f = (f##_t) NULL; char *str_##f = #f;

FUNC(int, __attribute__((stdcall)), RegisterHotKey, (void *wnd, int id, unsigned int mods, unsigned int vk))
FUNC(int, __attribute__((stdcall)), GetMessageA, (MSG *msg, void *wnd, unsigned int first, unsigned last))
FUNC(short, __attribute__((stdcall)), GetAsyncKeyState, (int vk))

#undef FUNC
