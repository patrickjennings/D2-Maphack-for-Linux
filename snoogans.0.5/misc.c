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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "d2pointers.h"
#include "misc.h"

#define LINE(offsets, x1, y1, x2, y2, i) offsets[i++] = x1; offsets[i++] = y1; offsets[i++] = x2; offsets[i++] = y2;

/*
 * we have to manually transform our strings because *nix
 * wchar_t is 4 bytes while windows uses 2 bytes only
 */

void
char_to_ms_wchar(char *str, ms_wchar_t *wstr)
{
  int i;
  for (i = 0; i < strlen(str); i++)
    {
      wstr[i] = (ms_wchar_t) (str[i] & 0xFF);
    }
  wstr[strlen(str)] = '\0';
}

void
ms_wchar_to_char(ms_wchar_t *wstr, char *str)
{
  int i;
  for (i = 0; wstr[i] != '\0'; i++)
    {
      str[i] = (char) (wstr[i] & 0xFF);
    }
  str[i] = '\0';
}

void
print_ingame(int color, const char *format, ...)
{
  va_list args;
  va_start(args, format);
  char msg[1024];
  vsprintf(msg, format, args);
  ms_wchar_t wmsg[strlen(msg) + 1];
  char_to_ms_wchar(msg, wmsg);
  D2CLIENT_print_game_string(wmsg, color);
}

void
screen_to_automap(POINT *p, int x, int y)
{
  p->x = ((x - y) / 2 / (*((int *) p_D2CLIENT_divisor))) - p_D2CLIENT_offset->x
      + 8;
  p->y = ((x + y) / 4 / (*((int *) p_D2CLIENT_divisor))) - p_D2CLIENT_offset->y
      - 8;
  if (D2CLIENT_get_automap_size())
    {
      --p->x;
      p->y += 5;
    }
}

void
draw_text(int x, int y, int color, int automap, int font, int center,
    char *format, ...)
{
  va_list args;
  va_start(args, format);
  char text[1024];
  vsprintf(text, format, args);
  ms_wchar_t wtext[strlen(text) + 1];
  char_to_ms_wchar(text, wtext);
  POINT p =
    { x, y, };
  if (automap)
    {
      if (!*p_D2CLIENT_automap_on)
        {
          return;
        }
      screen_to_automap(&p, x * 32, y * 32);
    }
  DWORD prev_font = D2WIN_set_text_size(font);
  if (center >= 0)
    {
      int width;
      DWORD file_no;
      D2WIN_get_text_width_file_no(wtext, &width, &file_no);
      p.x -= (width >> center);
    }
  D2WIN_draw_text(wtext, p.x, p.y, color, -1);
  D2WIN_set_text_size(prev_font);
}

void
draw_rectangle(int x, int y, int color, int automap)
{
  POINT p =
    { x, y };
  if (automap)
    {
      if (!*p_D2CLIENT_automap_on)
        {
          return;
        }
      screen_to_automap(&p, x * 32, y * 32);
    }
  D2GFX_draw_line(p.x - 3, p.y + 3, p.x + 3, p.y + 3, color, -1);
  D2GFX_draw_line(p.x - 3, p.y + 3, p.x - 3, p.y - 3, color, -1);
  D2GFX_draw_line(p.x - 3, p.y - 3, p.x + 3, p.y - 3, color, -1);
  D2GFX_draw_line(p.x + 3, p.y - 3, p.x + 3, p.y + 3, color, -1);
}

void
draw_cross(int x, int y, int color, int automap)
{
  POINT p =
    { x, y };
  if (automap)
    {
      if (!*p_D2CLIENT_automap_on)
        {
          return;
        }
      screen_to_automap(&p, x * 32, y * 32);
    }
  int lines[48];
  int i = 0;
  LINE(lines, p.x, p.y - 2, p.x + 4, p.y - 4, i)
  LINE(lines, p.x + 4, p.y - 4, p.x + 8, p.y - 2, i)
  LINE(lines, p.x + 8, p.y - 2, p.x + 4, p.y , i)
  LINE(lines, p.x + 4, p.y, p.x + 8, p.y + 2, i)
  LINE(lines, p.x + 8, p.y + 2, p.x + 4, p.y + 4, i)
  LINE(lines, p.x + 4, p.y + 4, p.x, p.y + 2, i)
  LINE(lines, p.x, p.y + 2, p.x - 4, p.y + 4, i)
  LINE(lines, p.x - 4, p.y + 4, p.x - 8, p.y + 2, i)
  LINE(lines, p.x - 8, p.y + 2, p.x - 4, p.y , i)
  LINE(lines, p.x - 4, p.y, p.x - 8, p.y - 2, i)
  LINE(lines, p.x - 8, p.y - 2, p.x - 4, p.y - 4, i)
  LINE(lines, p.x - 4, p.y - 4, p.x, p.y - 2, i)
  int j;
  for (j = 0; j < i; j += 4)
    {
      D2GFX_draw_line(lines[j], lines[j + 1], lines[j + 2], lines[j + 3],
          color, -1);
    }
}

void
draw_box(int x, int y, int width, int height, int color, int automap, int trans)
{
  POINT p =
    { x, y };
  if (automap)
    {
      if (!*p_D2CLIENT_automap_on)
        {
          return;
        }
      screen_to_automap(&p, x * 32, y * 32);
    }
  D2GFX_draw_rectangle(p.x, p.y, p.x + width, p.y + height, color, trans);
}

level *
get_level(act_misc *misc, DWORD level_no)
{
  level *level;
  for (level = misc->level_first; level != NULL; level = level->next)
    {
      if (level->level_no == level_no)
        {
          return level;
        }
    }
  return D2COMMON_get_level(misc, level_no);
}

automap_layer *
init_automap_layer(DWORD level_no)
{
  automap_layer2 *layer = D2COMMON_get_layer(level_no);
  return D2CLIENT_init_automap_layer(layer->layer_no);
}

int
ingame()
{
  unit_any *hero = D2CLIENT_get_player_unit();
  if (hero)
    if (hero->inventory)
      if (hero->path)
        if (hero->path->x)
          if (hero->path->room1)
            if (hero->path->room1->room2)
              if (hero->path->room1->room2->level)
                if (hero->path->room1->room2->level->level_no)
                  if (hero->act)
                    return 1;
  return 0;
}

int
valid_monster(unit_any *unit)
{
  if ((unit->mode == 0) || (unit->mode == 12))
    {
      return 0;
    }
  if (((unit->txtfile_no >= 110) && (unit->txtfile_no <= 113))
      || ((unit->txtfile_no == 608) && (unit->mode == 8)))
    {
      return 0;
    }
  if ((unit->txtfile_no == 68) && (unit->mode == 14))
    {
      return 0;
    }
  if (((unit->txtfile_no == 258) || (unit->txtfile_no == 261)) && (unit->mode
      == 14))
    {
      return 0;
    }
  if (D2COMMON_get_unit_stat(unit, 172, 0) == 2)
    {
      return 0;
    }
  DWORD false_positives[] =
    { 326, 327, 328, 329, 330, 410, 411, 412, 413, 414, 415, 416, 366, 406,
        351, 352, 353, 266, 408, 516, 517, 518, 519, 522, 523, 543, 543, 545 };
  int i;
  for (i = 0; i < 28; i++)
    {
      if (unit->txtfile_no == false_positives[i])
        {
          return 0;
        }
    }
  ms_wchar_t *wname = D2CLIENT_get_unit_name(unit);
  if (wname)
    {
      char name[512];
      ms_wchar_to_char(wname, name);
      if ((strcmp(name, "an evil force") == 0) || (strcmp(name, "dummy") == 0))
        {
          return 0;
        }
    }
  return 1;
}
