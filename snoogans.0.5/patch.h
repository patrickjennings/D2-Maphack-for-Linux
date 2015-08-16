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

#ifndef PATCH_H_
#define PATCH_H_

#include "d2structs.h"

extern char last_game_name[512];

extern unit_any *viewing_unit;

void
draw_automap();

void __attribute__((fastcall))
next_game_name_patch(vaddr, int __attribute__((stdcall))
(*call_back)(vaddr, DWORD, DWORD));

int
install_hooks();

int
remove_hooks();

#endif /* PATCH_H_ */
