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

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "d2pointers.h"
#include "kernel32.h"
#include "user32.h"

#include "debug.h"

const char *modules[] =
  { "D2Client.dll", "D2Common.dll", "D2Gfx.dll", "D2Lang.dll", "D2Win.dll",
      "D2Net.dll", "D2Game.dll", "D2Launch.dll", "Fog.dll", "BNClient.dll",
      "Storm.dll", "D2Cmp.dll", "D2Multi.dll" };

static int
read_line(FILE *fd, char *line)
{
  int n_bytes = 0;
  while (!feof(fd))
    {
      char chr = fgetc(fd);
      if (chr == EOF)
        {
          break;
        }
      if (chr == '\n')
        {
          n_bytes++;
          line[n_bytes - 1] = '\0';
          break;
        }
      n_bytes++;
      line[n_bytes - 1] = chr;
    }
  return n_bytes == 0 ? 0 : 1;
}

static int
get_module_path(const char *module, char *path)
{
  pid_t pid = getpid();
  char maps[512];
  sprintf(maps, "/proc/%i/maps", pid);
  FILE *fd = fopen(maps, "r");
  char line[512];
  while (read_line(fd, line))
    {
      if (strstr(line, module) != NULL)
        {
          strcpy(path, strchr(line, '/'));
          fclose(fd);
          return 1;
        }
      memset(line, 0, 512);
    }
  fclose(fd);
  return 0;
}

int
populate_kernel32_funcs()
{
  char module_kernel32[512];
  if (!get_module_path("kernel32.dll", module_kernel32))
    {
      printf("err: could not find location of kernel32.dll\n");
      return 0;
    }
  void *h = dlopen(module_kernel32, RTLD_LAZY);
  if (h == NULL)
    {
      printf("err: could not open %s\n", module_kernel32);
      return 0;
    }
  void **func;
  char **str;
  for (func = (void **) &_KERNEL32_FUNC_START, str
      = (char **) &_KERNEL32_STR_START; func <= (void **) &_KERNEL32_FUNC_END; func++, str++)
    {
      *func = dlsym(h, *str);
      if (*func == NULL)
        {
          printf("err: could not resolve symbol %s\n", *str);
          return 0;
        }
      DEBUG_DO(printf("%s: 0x%08X\n", *str, (vaddr) *func);)
    }
  return 1;
}

int
populate_user32_funcs()
{
  char module_user32[512];
  if (!get_module_path("user32.dll", module_user32))
    {
      printf("err: could not find location of user32.dll\n");
      return 0;
    }
  void *h = dlopen(module_user32, RTLD_LAZY);
  if (h == NULL)
    {
      printf("err: could not open %s\n", module_user32);
      return 0;
    }
  void **func;
  char **str;
  for (func = (void **) &_USER32_FUNC_START, str = (char **) &_USER32_STR_START; func
      <= (void **) &_USER32_FUNC_END; func++, str++)
    {
      *func = dlsym(h, *str);
      if (*func == NULL)
        {
          printf("err: could not resolve symbol %s\n", *str);
          return 0;
        }
      DEBUG_DO(printf("%s: 0x%08X\n", *str, (vaddr) *func);)
    }
  return 1;
}

int
populate_d2funcs()
{
  if (GetModuleHandleA == NULL || GetProcAddress == NULL || LoadLibraryA
      == NULL)
    {
      printf("err: necessary kernel32 functions missing\n");
      return 0;
    }
  void **func;
  for (func = (void **) &_D2FUNCS_START; func <= (void **) &_D2FUNCS_END; func++)
    {
      int index = (vaddr) *func & 0xff;
      int offset = (int) *func >> 8;
      void *module;
      if (((int) *func >> 8) > 0)
        {
          module = LoadLibraryA(modules[index]);
          if (module == NULL)
            {
              printf("err: could not get a handle for %s\n", modules[index]);
              return 0;
            }
          *func = module + offset;
          DEBUG_DO(printf("populated function pointer (0x%08X) to %s (0x%08X) + 0x%08X\n", (vaddr) *func, modules[index], (vaddr) module, offset);)
        }
      else
        {
          module = GetModuleHandleA(modules[index]);
          if (module == NULL)
            {
              printf("err: could not get a handle for %s\n", modules[index]);
              return 0;
            }
          *func = GetProcAddress(module, -offset);
          if (*func == NULL)
            {
              printf(
                  "err: could not resolve exported function with ordinal %i\n",
                  -offset);
              return 0;
            }
          DEBUG_DO(printf("populated function pointer (0x%08X) to %s (0x%08X) -> %i\n", (vaddr) *func, modules[index], (vaddr) module, -offset);)
        }

    }
  return 1;
}

int
populate_d2vars()
{
  if (GetModuleHandleA == NULL || GetProcAddress == NULL || LoadLibraryA
      == NULL)
    {
      printf("err: necessary kernel32 functions missing\n");
      return 0;
    }
  void **var;
  for (var = (void **) &_D2VARS_START; var <= (void **) &_D2VARS_END; var++)
    {
      int index = (vaddr) *var & 0xff;
      int offset = (vaddr) *var >> 8;
      void *module = LoadLibraryA(modules[index]);
      if (module == NULL)
        {
          printf("err: could not get a handle for %s\n", modules[index]);
          return 0;
        }
      *var = module + offset;
      DEBUG_DO(printf("populated variable pointer (0x%08X) to %s (0x%08X) + 0x%08X\n", (vaddr) *var, modules[index], (vaddr) module, offset);)
    }
  return 1;
}
