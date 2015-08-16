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

#ifndef MISC_H_
#define MISC_H_

#include "d2structs.h"
#include "types.h"

typedef enum
{
  D2FONT_WHITE,
  D2FONT_RED,
  D2FONT_GREEN,
  D2FONT_BLUE,
  D2FONT_GOLD,
  D2FONT_GREY,
  D2FONT_BLACK,
  D2FONT_ORANGE = 8,
  D2FONT_YELLOW = 9,
  D2FONT_PURPLE = 11
} D2FONT_COLOR;

typedef enum
{
  D2GFX_WHITE = 0xFF,
  D2GFX_RED = 0x0A,
  D2GFX_GREEN = 0x7D,
  D2GFX_BLUE = 0x91,
  D2GFX_GOLD = 0x5A,
  D2GFX_GREY = 0xBC,
  D2GFX_BLACK = 0x00,
  D2GFX_ORANGE = 0x60,
  D2GFX_YELLOW = 0x6F,
  D2GFX_PURPLE = 0xE1
} D2GFX_COLOR;

typedef enum
{
  D2FONT_SMALL = 6,
  D2FONT_STANDARD = 0,
  D2FONT_HUGE = 5,
  D2FONT_SPECIAL = 1
} D2FONT_SIZE;

#define FONT_COLOR(c) ((char[]) { 0xFF, 'c', #c[0], '\0' })

void
char_to_ms_wchar(char *, ms_wchar_t *);

void
ms_wchar_to_char(ms_wchar_t *, char *);

void
print_ingame(int, const char *, ...);

void
draw_text(int, int, int, int, int, int, char *, ...);

void
draw_rectangle(int, int, int, int);

void
draw_cross(int, int, int, int);

void
draw_box(int, int, int, int, int, int, int);

level *
get_level(act_misc *, DWORD);

automap_layer *
init_automap_layer(DWORD);

int
ingame();

int
valid_monster(unit_any *);

#endif /* MISC_H_ */
