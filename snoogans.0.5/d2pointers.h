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

#pragma once

#ifndef D2POINTERS_H_
#define D2POINTERS_H_

#include "d2structs.h"
#include "types.h"

#define FUNC(r, a, m, f, p) typedef r a (*m##_##f##_t)p; extern m##_##f##_##t m##_##f;
#define VAR(t, m, v) typedef t m##_##v##_t; extern m##_##v##_t *p_##m##_##v;

/* function pointers */

/* D2Client.dll */

FUNC(unit_any *, __attribute__((stdcall)), D2CLIENT, get_player_unit, ())
FUNC(void, __attribute__((stdcall)), D2CLIENT, reveal_automap_room, (room1 *room1, DWORD clip_flag, automap_layer *layer))
FUNC(automap_layer *, __attribute__((regparm(3))), D2CLIENT, init_automap_layer, (DWORD layer_no))
FUNC(void, __attribute__((stdcall)), D2CLIENT, print_game_string, (ms_wchar_t *message, int color))
FUNC(DWORD, __attribute__((stdcall)), D2CLIENT, get_automap_size, ())
FUNC(ms_wchar_t *, __attribute__((regparm(3))), D2CLIENT, get_unit_name, (unit_any *))
FUNC(void, __attribute__((fastcall)), D2CLIENT, add_automap_cell, (automap_cell *cell, automap_cell **node))
FUNC(automap_cell *, __attribute__((fastcall)), D2CLIENT, new_automap_cell, ())
FUNC(game_info *, __attribute__((stdcall)), D2CLIENT, get_game_info, ())
FUNC(unit_any *, __attribute__((stdcall)), D2CLIENT, get_selected_unit, ())
FUNC(DWORD, __attribute__((regparm(3))), D2CLIENT, get_ui_var, (DWORD var_no))
FUNC(DWORD, __attribute__((fastcall)), D2CLIENT, set_ui_var, (DWORD var_no, DWORD howset, DWORD unknown))
FUNC(unit_any *, __attribute__((fastcall)), D2CLIENT, find_server_side_unit, (DWORD id, DWORD type))

/* D2Gfx.dll */

FUNC(void, __attribute__((stdcall)), D2GFX, draw_line, (int x1, int y1, int x2, int y2, DWORD color, DWORD unk))
FUNC(void, __attribute__((stdcall)), D2GFX, draw_rectangle, (int x1, int y1, int x2, int y2, DWORD color, DWORD trans))

/* D2Win.dll */

FUNC(DWORD, __attribute__((fastcall)), D2WIN, set_text_size, (DWORD size))
FUNC(void, __attribute__((fastcall)), D2WIN, draw_text, (ms_wchar_t *text, int x, int y, int color, DWORD unk))
FUNC(DWORD, __attribute__((fastcall)), D2WIN, get_text_width_file_no, (ms_wchar_t *text, int *width, DWORD *file_no))
FUNC(void, __attribute__((fastcall)), D2WIN, select_edit_box_text, (vaddr box, ms_wchar_t *text))
FUNC(void *, __attribute__((fastcall)), D2WIN, set_control_text, (vaddr box, ms_wchar_t *text))
FUNC(void, __attribute__((fastcall)), D2WIN, set_edit_box_proc, (vaddr box, int __attribute__((stdcall)) (*call_back)(vaddr, DWORD, DWORD)))

/* D2Lang.dll */

FUNC(ms_wchar_t *, __attribute__((fastcall)), D2LANG, get_local_text, (DWORD local_text_no))

/* D2Common.dll */

FUNC(void, __attribute__((stdcall)), D2COMMON, add_room_data, (act *act, DWORD level_no, int x, int y, room1 *room1))
FUNC(void, __attribute__((stdcall)), D2COMMON, remove_room_data, (act *act, DWORD level_no, int x, int y, room1 *room1))
FUNC(automap_layer2 *, __attribute__((fastcall)), D2COMMON, get_layer, (DWORD level_no))
FUNC(level *, __attribute__((fastcall)), D2COMMON, get_level, (act_misc *misc, DWORD level_no))
FUNC(void, __attribute__((stdcall)), D2COMMON, init_level, (level *level))
FUNC(object_txt *, __attribute__((stdcall)), D2COMMON, get_object_txt, (DWORD obj_no))
FUNC(DWORD, __attribute__((stdcall)), D2COMMON, get_unit_stat, (unit_any *unit, DWORD stat, DWORD stat2))

#define _D2FUNCS_START D2CLIENT_get_player_unit
#define _D2FUNCS_END D2COMMON_get_unit_stat

#undef FUNC

/* variable pointers */

/* D2Client.dll */

VAR(automap_layer *, D2CLIENT, automap_layer)
VAR(automap_layer *, D2CLIENT, first_automap_layer)
VAR(int, D2CLIENT, divisor)
VAR(POINT, D2CLIENT, offset)
VAR(DWORD, D2CLIENT, automap_on)
VAR(unit_any *, D2CLIENT, player_unit)

/* D2Win.dll */

VAR(DWORD, D2WIN, first_control)

#define _D2VARS_START p_D2CLIENT_automap_layer
#define _D2VARS_END p_D2WIN_first_control

#undef VAR

#endif /* D2POINTERS_H_ */
