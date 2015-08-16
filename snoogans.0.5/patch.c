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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "d2pointers.h"
#include "kernel32.h"
#include "maphack.h"
#include "misc.h"
#include "patch.h"
#include "stubs.h"
#include "types.h"

#include "debug.h"

#include "version.h"

#define PAGE_ALIGN(addr) (addr & ~0xfff)

char last_game_name[512];

unit_any *viewing_unit;

#define i386_CALL 0xE8
#define i386_JUMP 0xE9

typedef struct
{
  unsigned char type;
  const char *module;
  vaddr offset;
  size_t size;
  void *func;
  unsigned char *backup;
} hook;

#define N_HOOKS 8

hook
    hooks[] =
      {

#ifdef _VERSION_1_13d /* updated */
            { i386_CALL, "D2Client.dll", 0x7349E, 5, draw_automap, NULL}, // updated
            { i386_JUMP, "D2Client.dll", 0x1D7B4, 6, draw_ingame_STUB, NULL}, // updated
            { i386_CALL, "D2Client.dll", 0x96736, 6, item_name_patch_STUB, NULL}, // updated
            { i386_CALL, "D2Multi.dll", 0xB5E9, 5, next_game_name_patch, NULL}, // updated
            { i386_CALL, "D2Multi.dll", 0xADAB, 5, next_game_name_patch, NULL}, // updated
            { i386_CALL, "D2Client.dll", 0x997B2, 6, view_inventory_patch_1, NULL}, // updated
            { i386_CALL, "D2Client.dll", 0x98E84, 6, view_inventory_patch_2, NULL}, // updated
            { i386_CALL, "D2Client.dll", 0x97E3F, 5, view_inventory_patch_3, NULL}, // updated
            //{ i386_CALL, "D2Client.dll", 0x83301, 5, packet_received_STUB, NULL }, // updated 0x6FB33301
            //{ i386_JUMP, "D2Client.dll", 0xD13C, 5, packet_sent_STUB, NULL } // updated 6FB662B1:  e8 86 6e f5 ff  call 0x6FABD13C

#elif defined _VERSION_1_13c /* updated */
            { i386_CALL, "D2Client.dll", 0x626FE, 5, draw_automap, NULL}, /* updated 0x6FB126FE */
            { i386_JUMP, "D2Client.dll", 0xC3DB4, 6, draw_ingame_STUB, NULL}, /* updated 0x6FB73DB4 */
            { i386_CALL, "D2Client.dll", 0x92366, 6, item_name_patch_STUB, NULL}, /* updated 0x6FB42366 */
            { i386_CALL, "D2Multi.dll", 0x14D29, 5, next_game_name_patch, NULL}, /* updated 0x6F9E4D29 */
            { i386_CALL, "D2Multi.dll", 0x14A0B, 5, next_game_name_patch, NULL}, /* updated 0x6F9E4A0B */
            { i386_CALL, "D2Client.dll", 0x953E2, 6, view_inventory_patch_1, NULL}, /* updated 0x6FB453E2 */
            { i386_CALL, "D2Client.dll", 0x94AB4, 6, view_inventory_patch_2, NULL}, /* updated 0x6FB44AB4 */
            { i386_CALL, "D2Client.dll", 0x93A6F, 5, view_inventory_patch_3, NULL} /* updated 0x6FB43A6F */

#elif defined _VERSION_1_12 /* from McGod's mMap (modified) */
            { i386_CALL, "D2Client.dll", 0x4063E, 5, draw_automap, NULL },
            { i386_JUMP, "D2Client.dll", 0x90524, 6, draw_ingame_STUB, NULL },
            { i386_CALL, "D2Client.dll", 0xAF242, 6, view_inventory_patch_1, NULL },
            { i386_CALL, "D2Client.dll", 0xAE914, 6, view_inventory_patch_2, NULL },
            { i386_CALL, "D2Client.dll", 0xAD93F, 5, view_inventory_patch_3, NULL },
            { i386_CALL, "D2Client.dll", 0xAC236, 6, item_name_patch_STUB, NULL },
            { i386_CALL, "D2Multi.dll", 0x14959, 5, next_game_name_patch, NULL },
            { i386_CALL, "D2Multi.dll", 0x1463B, 5, next_game_name_patch, NULL }

#else

#error NO VERSION SPECIFIED

#endif

    };

void
draw_automap()
{
  reveal_automap();
  draw_monsters();
  draw_automap_text();
  if (strlen(D2CLIENT_get_game_info()->game_ip) > 0)
    {
      draw_text(780, 122, D2FONT_GOLD, 0, D2FONT_SPECIAL, 0, "%s",
          D2CLIENT_get_game_info()->game_ip);
    }
}

void
draw_ingame()
{
  draw_text(400, 540, D2FONT_WHITE, 0, D2FONT_STANDARD, 1, "%s",
      "snoogans.so v 0.5");
}

void __attribute__((fastcall))
item_name_patch(unit_any *item, ms_wchar_t *wname)
{
  if (!item || !wname)
    {
      return;
    }
  char tmp[512];
  ms_wchar_to_char(wname, tmp);
  char line1[512], *line2;
  strcpy(line1, strtok(tmp, "\n"));
  line2 = strtok(NULL, "\n");
  int sockets = D2COMMON_get_unit_stat(item, 194, 0);
  if (sockets > 0)
    {
      char c_sockets[5];
      sprintf(c_sockets, " (%i)", sockets);
      strcat(line1, c_sockets);
    }
  if (item->item_data->flags & 0x400000)
    {
      char eth[] = " (Eth)";
      strcat(line1, eth);
    }
  char name[1024];
  sprintf(name, "(%i) ", item->item_data->level);
  strcat(name, line1);
  if (line2)
    {
      strcat(name, "\n");
      strcat(name, line2);
    }
  char_to_ms_wchar(name, wname);
}

void __attribute__((fastcall))
next_game_name_patch(vaddr box, int __attribute__((stdcall))
(*call_back)(vaddr, DWORD, DWORD))
{
  if (strlen(last_game_name) == 0)
    {
      return;
    }
  ms_wchar_t wname[512];
  char_to_ms_wchar(last_game_name, wname);
  D2WIN_set_control_text(box, wname);
  D2WIN_select_edit_box_text(box, wname);
  D2WIN_set_edit_box_proc(box, call_back);
}

unit_any *
get_view_unit()
{
  if (!viewing_unit)
    {
      return D2CLIENT_get_player_unit();
    }
  return viewing_unit;
}

unit_any *
view_inventory_patch()
{
  unit_any *unit = get_view_unit();
  unit_any *me = D2CLIENT_get_player_unit();
  if (unit->id == me->id)
    {
      return me;
    }
  draw_text(560, 300, D2FONT_WHITE, 0, D2FONT_SPECIAL, 1, "%s",
      unit->player_data->name);
  return get_view_unit();
}

static int
install_hook(const char *module, vaddr offset, vaddr func, size_t bytes,
    unsigned char inst, unsigned char **backup)
{
  void *h = LoadLibraryA(module);
  if (h == NULL)
    {
      printf("err: could not get a handle for %s\n", module);
      return 0;
    }
  vaddr addr = (vaddr) h + offset;
  unsigned char hook[bytes];
  memset(hook, 0x90, bytes);
  hook[0] = inst;
  *(vaddr *) &hook[1] = func - (addr + 5);
  DEBUG_DO(printf("installing hook at 0x%08X to func (0x%08X)\n", addr, func)); DEBUG_DO(printf("ASM: ");)
  DEBUG_DO(inst == i386_CALL ? printf("call ") : printf("jmp ");)
  DEBUG_DO(printf("0x%08X\n", func - (addr + 5));)
  DEBUG_DO(printf("enabling write access to page(s) starting from 0x%08X\n", PAGE_ALIGN(addr));)
  if (mprotect((void *) PAGE_ALIGN(addr), bytes, PROT_READ | PROT_WRITE
      | PROT_EXEC))
    {
      printf("err: could not enable write access to page(s)\n");
      return 0;
    }
  *backup = (unsigned char *) malloc(bytes);
  memcpy((void *) *backup, (void *) addr, bytes);
  DEBUG_DO(printf("saved original code\n");)
  memcpy((void *) addr, (void *) hook, bytes);
  DEBUG_DO(printf("patched to:\n");
      int i;
      for (i = 0; i < bytes; i++)
        {
          printf("0x%02X\n", hook[i]);
        })
  DEBUG_DO(printf("disabling write access to page(s) starting from 0x%08X\n", PAGE_ALIGN(addr));)
  if (mprotect((void *) PAGE_ALIGN(addr), bytes, PROT_READ | PROT_EXEC))
    {
      printf("err: could not disable write access to page(s)\n");
      return 0;
    }
  DEBUG_DO(printf("done\n");)
  return 1;
}

int
install_hooks()
{
  int i;
  for (i = 0; i < N_HOOKS; i++)
    {
      if (!install_hook(hooks[i].module, hooks[i].offset,
          (vaddr) hooks[i].func, hooks[i].size, hooks[i].type, &hooks[i].backup))
        {
          return 0;
        }
    }
  return 1;
}

static int
remove_hook(const char *module, vaddr offset, size_t bytes,
    unsigned char *backup)
{
  void *h = LoadLibraryA(module);
  if (h == NULL)
    {
      printf("err: could not get a handle for %s\n", module);
      return 0;
    }
  vaddr addr = (vaddr) h + offset;
  DEBUG_DO(printf("enabling write access to page(s) starting from 0x%08X\n", PAGE_ALIGN(addr));)
  if (mprotect((void *) PAGE_ALIGN(addr), bytes, PROT_READ | PROT_WRITE
      | PROT_EXEC))
    {
      printf("err: could not enable write access to page(s)\n");
      return 0;
    }
  memcpy((void *) addr, (void *) backup, bytes);
  DEBUG_DO(printf("restored original code:\n");
      int i;
      for (i = 0; i < bytes; i++)
        {
          printf("0x%02X\n", backup[i]);
        })
  free(backup);
  DEBUG_DO(printf("disabling write access to page(s) starting from 0x%08X\n", PAGE_ALIGN(addr));)
  if (mprotect((void *) PAGE_ALIGN(addr), bytes, PROT_READ | PROT_EXEC))
    {
      printf("err: could not disable write access to page(s)\n");
      return 0;
    }
  DEBUG_DO(printf("done\n");)
  return 1;
}

int
remove_hooks()
{
  int i;
  for (i = 0; i < N_HOOKS; i++)
    {
      if (hooks[i].backup)
        {
          if (!remove_hook(hooks[i].module, hooks[i].offset, hooks[i].size,
              hooks[i].backup))
            {
              return 0;
            }
        }
    }
  return 1;
}
