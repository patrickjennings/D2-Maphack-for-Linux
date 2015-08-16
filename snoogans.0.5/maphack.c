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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "d2pointers.h"
#include "d2structs.h"
#include "kernel32.h"
#include "misc.h"
#include "patch.h"
#include "types.h"
#include "user32.h"

#include "debug.h"

#define CLEAR(a, n, t) memset(a, 0, n * sizeof(t))

typedef struct level_info
{
  int act_no;
  DWORD level_id;
  int x;
  int y;
} level_info;

#define PRINT(info) printf("level_info.act_no: %i\n", info.act_no); printf("level_info.level_id: %i\n", info.level_id); printf("level_info.x: %i\n", info.x); printf("level_info.y: %i\n", info.y);

int n_info_level = 0;
level_info *info_level = NULL;

#define INIT(info) info = NULL; n_##info = 0;
#define ADD(type, info, i) info = realloc(info, ++n_##info * sizeof(type)); memcpy(info + n_##info - 1, &i, sizeof(type));
#define REMOVE_ALL(info) if (info) free(info); info = NULL; n_##info = 0;
#define SIZE(info) n_##info
#define ELEMENT(info, i) info[i]

pthread_t mh_thread;

int area_local_id[] =
  { 0, 5055, 5054, 5053, 5052, 5051, 5050, 5049, 3714, 5047, 5046, 5045, 5044,
      5043, 5042, 5041, 5040, 5039, 5038, 5037, 5036, 5035, 5034, 5033, 5032,
      5031, 5030, 5029, 5028, 5027, 5026, 5025, 5024, 5023, 5022, 5021, 5020,
      5019, 5018, 788, 852, 851, 850, 849, 848, 847, 846, 845, 844, 843, 842,
      841, 840, 839, 838, 837, 836, 835, 834, 833, 832, 831, 830, 829, 828,
      827, 826, 826, 826, 826, 826, 826, 826, 825, 824, 820, 819, 818, 817,
      816, 815, 814, 813, 812, 810, 811, 809, 808, 806, 805, 807, 804, 845,
      844, 803, 802, 801, 800, 799, 798, 797, 796, 795, 790, 792, 793, 794,
      791, 789, 22646, 22647, 22648, 22649, 22650, 22651, 22652, 22653, 22654,
      22655, 22656, 22657, 22660, 21865, 21866, 21867, 22658, 22659, 22662,
      22663, 22664, 22665, 22667, 22666, 11155, 11156, 11157, 5018 };

int town_levels[] =
  { 1, 40, 75, 103, 109, 137 };

int revealed_act[5];

int loaded = 0;

int reset = 1;

void
draw_presets(room2 *room2)
{
  preset_unit *unit;
  for (unit = room2->preset; unit != NULL; unit = unit->next)
    {
      int cell_no = -1;
      switch (unit->type)
        {
      case 1:
        {
          switch (unit->txt_file_no)
            {
          case 256:
            {
              cell_no = 300;
              break;
            }
          case 745:
            {
              cell_no = 745;
              break;
            }
            }
        }
      case 2:
        {
          switch (unit->txt_file_no)
            {
          case 580:
            {
              if (room2->level->level_no == 79)
                {
                  cell_no = 318;
                }
              break;
            }
          case 371:
            {
              if (room2->level->level_no == 25)
                {
                  cell_no = 301;
                }
              break;
            }
          case 152:
            {
              cell_no = 300;
              break;
            }
          case 460:
            {
              cell_no = 1468;
              break;
            }
          case 402:
            {
              if (room2->level->level_no == 46)
                {
                  cell_no = 0;
                }
              break;
            }
          case 267:
            {
              if (room2->level->level_no != 75 && room2->level->level_no != 103)
                {
                  cell_no = 0;
                }
              break;
            }
          case 367:
            {
              if (room2->level->level_no == 107)
                {
                  cell_no = 376;
                }
              break;
            }
            }
          if (cell_no == -1)
            {
              if (unit->txt_file_no > 572)
                {
                  cell_no = 0;
                }
              else
                {
                  object_txt *obj = D2COMMON_get_object_txt(unit->txt_file_no);
                  if (obj->automap == 310)
                    {
                      cell_no = obj->subclass & 1 ? 310 : 0;
                    }
                  else
                    {
                      cell_no = obj->automap;
                    }
                }
            }
          break;
        }
      case 5:
        {
          level_info info =
            { 0 };
          room_tile *tile;
          if (room2->room_tiles == NULL)
            {
              break;
            }
          for (tile = room2->room_tiles; tile != NULL; tile = tile->next)
            {
              if (*(tile->num) == unit->txt_file_no)
                {
                  info.level_id = tile->room2->level->level_no;
                  break;
                }
            }
          if (info.level_id == 0)
            {
              break;
            }
          unit_any *hero = D2CLIENT_get_player_unit();
          info.act_no = hero->act_no;
          info.x = unit->x + (room2->x * 5) - (8 << 1);
          info.y = unit->y + (room2->y * 5) - 10;
          DEBUG_DO(printf("add level info #%i\n", n_info_level);)
          DEBUG_DO(PRINT(info);)
          ADD(level_info, info_level, info)
          break;
        }
        }
      if (cell_no > 0)
        {
          automap_cell *cell = D2CLIENT_new_automap_cell();
          cell->cell_no = cell_no;
          int x = unit->x + room2->x * 5;
          int y = unit->y + room2->y * 5;
          cell->pixel_x = ((x - y) * 16) / 10 + 1;
          cell->pixel_y = ((x + y) * 8) / 10 - 3;
          D2CLIENT_add_automap_cell(cell,
              &((*p_D2CLIENT_automap_layer)->objects));
        }
    }
}

int
reveal_level(level *level)
{
  DEBUG_DO(printf("init automap layer...\n");)
  if (!init_automap_layer(level->level_no))
    {
      printf("err: could not init automap layer for level %i\n",
          level->level_no);
      return 0;
    }
  DEBUG_DO(printf("automap_layer: 0x%08X\n", (vaddr) *p_D2CLIENT_automap_layer);)
  DEBUG_DO(printf("get player struct...\n");)
  unit_any *unit = D2CLIENT_get_player_unit();
  if (unit == NULL)
    {
      printf("err: could not get player unit\n");
      return 0;
    }
  DEBUG_DO(printf("unit: 0x%08X\n", (vaddr) unit);)
  room2 *room2;
  for (room2 = level->room2_first; room2 != NULL; room2 = room2->next)
    {
      DEBUG_DO(printf("room2: 0x%08X\n", (vaddr) room2);)
      int remove = 0;
      if (room2->room1 == NULL)
        {
          DEBUG_DO(printf("add room data...\n");)
          D2COMMON_add_room_data(level->misc->act, level->level_no, room2->x,
              room2->y, unit->path->room1);
          if (room2->room1 == NULL)
            {
              printf("err: could not add room data\n");
              continue;
            }
          remove = 1;
        }
      DEBUG_DO(printf("room1: 0x%08X\n", (vaddr) room2->room1);)
      DEBUG_DO(printf("reveal room...\n");)
      D2CLIENT_reveal_automap_room(room2->room1, 1, *p_D2CLIENT_automap_layer);
      DEBUG_DO(printf("draw presets...\n");)
      draw_presets(room2);
      if (remove)
        {
          DEBUG_DO(printf("remove room data...\n");)
          D2COMMON_remove_room_data(level->misc->act, level->level_no,
              room2->x, room2->y, unit->path->room1);
        }
    }
  DEBUG_DO(printf("init automap layer back...\n");)
  init_automap_layer(unit->path->room1->room2->level->level_no);
  return 1;
}

int
reveal_act()
{
  DEBUG_DO(printf("get player unit...\n");)
  unit_any *unit = D2CLIENT_get_player_unit();
  DEBUG_DO(printf("unit: 0x%08X\n", (vaddr) unit);)
  if (unit == NULL)
    {
      return 0;
      printf("err: could not get player unit\n");
    }
  int n_level;
  for (n_level = town_levels[unit->act_no] + 1; n_level
      < town_levels[unit->act_no + 1]; n_level++)
    {
      if (n_level == 132)
        {
          continue;
        }
      DEBUG_DO(printf("get level struct...\n");)
      level *level = get_level(unit->act->misc, n_level);
      DEBUG_DO(printf("level: 0x%08X\n", (vaddr) level);)
      if (level == NULL)
        {
          printf("err: could not get a valid level struct\n");
          continue;
        }
      if (level->room2_first == NULL)
        {
          DEBUG_DO(printf("try to init level...\n");)
          D2COMMON_init_level(level);
          if (level->room2_first == NULL)
            {
              printf("err: could not init level %i\n", n_level);
              continue;
            }
        }
      DEBUG_DO(printf("reveal level %i...\n", n_level);)
      if (!reveal_level(level))
        {
          printf("err: could not reveal level %i\n", n_level);
        }
    }
  DEBUG_DO(printf("revealed all levels\n");)
  DEBUG_DO(printf("revealed act %i\n", unit->act_no + 1);)
  return 1;
}

void
reveal_automap()
{
  unit_any *hero = *p_D2CLIENT_player_unit;
  if (!hero)
    {
      return;
    }
  if (!revealed_act[hero->act_no])
    {
      manage_threads(SuspendThread);
      if (reveal_act())
        {
          revealed_act[hero->act_no] = 1;
          print_ingame(D2FONT_GOLD, "[snoogans] %s%s %i", FONT_COLOR(0),
              "revealed act", hero->act_no + 1);
        }
      else
        {
          print_ingame(D2FONT_GOLD, "[snoogans] %s%s %i", FONT_COLOR(1),
              "failed to reveal act", hero->act_no + 1);
        }
      manage_threads(ResumeThread);
    }
}

void
draw_monsters()
{
  unit_any *player = D2CLIENT_get_player_unit();
  if (player == NULL)
    {
      printf("err: could not get player struct\n");
      return;
    }
  int level_no = player->path->room1->room2->level->level_no;
  if (level_no == 1 || level_no == 40 || level_no == 75 || level_no == 103
      || level_no == 109)
    {
      return;
    }
  room1 *room1;
  for (room1 = player->act->room1; room1 != NULL; room1 = room1->next)
    {
      unit_any *unit;
      for (unit = room1->unit_first; unit != NULL; unit = unit->list_next)
        {
          if (!valid_monster(unit))
            {
              continue;
            }
          if (unit->type == 1)
            {
              if (unit->monster_data->boss & 1 && !(unit->monster_data->champ
                  & 1))
                {
                  draw_cross(unit->path->x, unit->path->y, D2GFX_WHITE, 1);
                  if (unit->monster_data->boss & 1
                      && !(unit->monster_data->champ & 1))
                    {
                      ms_wchar_t *wname = D2CLIENT_get_unit_name(unit);
                      if (wname)
                        {
                          char name[512];
                          ms_wchar_to_char(wname, name);
                          draw_text(unit->path->x - 6, unit->path->y - 6,
                              D2FONT_GOLD, 1, D2FONT_SMALL, 1, "%s", name);
                        }
                    }
                }
              /* check immunities */

              /* physical */
              else if (D2COMMON_get_unit_stat(unit, 36, 0) >= 100)
                {
                  draw_cross(unit->path->x, unit->path->y, D2GFX_GOLD, 1);
                }
              /* cold */
              else if (D2COMMON_get_unit_stat(unit, 43, 0) >= 100)
                {
                  draw_cross(unit->path->x, unit->path->y, D2GFX_BLUE, 1);
                }
              /* fire */
              else if (D2COMMON_get_unit_stat(unit, 39, 0) >= 100)
                {
                  draw_cross(unit->path->x, unit->path->y, D2GFX_RED, 1);
                }
              /* light */
              else if (D2COMMON_get_unit_stat(unit, 41, 0) >= 100)
                {
                  draw_cross(unit->path->x, unit->path->y, D2GFX_YELLOW, 1);
                }
              /* poison */
              else if (D2COMMON_get_unit_stat(unit, 45, 0) >= 100)
                {
                  draw_cross(unit->path->x, unit->path->y, D2GFX_GREEN, 1);
                }
              /* magic */
              else if (D2COMMON_get_unit_stat(unit, 37, 0) >= 100)
                {
                  draw_cross(unit->path->x, unit->path->y, D2GFX_ORANGE, 1);
                }
              else
                {
                  draw_cross(unit->path->x, unit->path->y, D2GFX_WHITE, 1);
                }
            }
          else if (unit->type == 3)
            {
              int missile_color = (player->id == unit->owner_id) ? D2GFX_GREEN
                  : D2GFX_RED;
              draw_box(unit->path->x - 3, unit->path->y - 3, 3, 3,
                  missile_color, 1, 5);
            }
        }
    }
}

void
draw_automap_text()
{
  unit_any *player = D2CLIENT_get_player_unit();
  int i;
  for (i = 0; i < SIZE(info_level); i++)
    {
      level_info info = ELEMENT(info_level, i);
      if (info.act_no != player->act_no)
        {
          continue;
        }
      ms_wchar_t *local_text = D2LANG_get_local_text(
          area_local_id[info.level_id]);
      if (local_text)
        {
          int font = D2FONT_WHITE;
          char level_name[512];
          ms_wchar_to_char(local_text, level_name);
          if (info.level_id == player->act->misc->staff_tomb_level)
            {
              font = D2FONT_GREEN;
            }
          draw_text(info.x, info.y, font, 1, D2FONT_SMALL, 1, "%s", level_name);
        }
    }
}

void *
maphack(void *arg)
{
  loaded = 1;
  if (ingame())
    {
      print_ingame(D2FONT_GOLD, "[snoogans] %s%s", FONT_COLOR(0),
          "loaded snoogans.so v 0.5");
    }
  CLEAR(revealed_act, 5, int);
  while (loaded)
    {
      usleep(100000);
      if (!ingame() && *p_D2WIN_first_control)
        {
          CLEAR(revealed_act, 5, int);
          REMOVE_ALL(info_level)
          reset = 1;
        }
      else if (ingame())
        {
          if (!D2CLIENT_get_ui_var(0x01))
            {
              viewing_unit = NULL;
            }
          if (viewing_unit && viewing_unit->id)
            {
              if (!viewing_unit->inventory)
                {
                  viewing_unit = NULL;
                  D2CLIENT_set_ui_var(0x01, 1, 0);
                }
              else if (!D2CLIENT_find_server_side_unit(viewing_unit->id,
                  viewing_unit->type))
                {
                  viewing_unit = NULL;
                  D2CLIENT_set_ui_var(0x01, 1, 0);
                }
              else if (viewing_unit->mode == 0 || viewing_unit->mode == 17)
                {
                  viewing_unit = NULL;
                  D2CLIENT_set_ui_var(0x01, 1, 0);
                }
            }
          unit_any *unit = D2CLIENT_get_selected_unit();
          if (unit && unit->inventory && unit->mode != 0 && unit->mode != 17
              && unit->type == 0)
            {
              if (!D2CLIENT_get_ui_var(0x01))
                {
                  viewing_unit = unit;
                }
            }
          game_info *info = D2CLIENT_get_game_info();
          if (info && reset)
            {
              strcpy(last_game_name, info->game_name);
              reset = 0;
            }
        }
    }
  if (ingame())
    {
      print_ingame(D2FONT_GOLD, "[snoogans] %s%s", FONT_COLOR(0), "unloaded");
    }
  REMOVE_ALL(info_level)
  return NULL;
}

void
stop_maphack()
{
  if (!loaded)
    {
      return;
    }
  loaded = 0;
  pthread_join(mh_thread, NULL);
}
