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

#ifndef USER32_H_
#define USER32_H_

#include "types.h"

#define VK_NUMPAD0 0x60

#define WM_HOTKEY 0x312

typedef struct MSG
{
  void *wnd;
  unsigned int message;
  DWORD w_param;
  DWORD l_param;
  DWORD time;
  POINT p;
} MSG;

#define FUNC(r, a, f, p) extern r (*f)p a; extern char *str_##f;

FUNC(int, __attribute__((stdcall)), RegisterHotKey, (void *wnd, int id, unsigned int mods, unsigned int vk))
FUNC(int, __attribute__((stdcall)), GetMessageA, (MSG *msg, void *wnd, unsigned int first, unsigned last))
FUNC(short, __attribute__((stdcall)), GetAsyncKeyState, (int vk))

#undef FUNC

#define _USER32_FUNC_START RegisterHotKey
#define _USER32_FUNC_END GetAsyncKeyState

#define _USER32_STR_START str_RegisterHotKey
#define _USER32_STR_END str_GetAsyncKeyState

#endif /* USER32_H_ */
