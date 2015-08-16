/*
 * Copyright (C) 2010 gonzoj
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

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "proc_utils.h"

#define PID_MAX 5

char *
read_line(FILE *file)
{
  static char *line = NULL;

  if (line)
    {
      free(line);
      line = NULL;
    }

  int i = 0;
  while (!feof(file))
    {
      if (i % 512 == 0)
        {
          line = (char *) realloc(line, i + 512);
          if (line == NULL)
            {
              printf("err: %s\n", strerror(errno));
              return NULL;
            }
        }

      char chr = fgetc(file);
      if (chr == EOF)
        {
          break;
        }
      if (chr == '\n')
        {
          line[i] = '\0';
          break;
        }
      line[i++] = chr;
    }

  return line;
}

int
proc_get_object_base(const char *object, vaddr_t *base)
{
  char *path =
      (char *) malloc(strlen("/proc/") + PID_MAX + strlen("/maps") + 1);
  if (path == NULL)
    {
      printf("err: %s\n", strerror(errno));
      return 0;
    }
  sprintf(path, "/proc/%u/maps", pid);
  FILE *maps = fopen(path, "r");
  if (maps == NULL)
    {
      printf("err: %s\n", strerror(errno));
      free(path);
      return 0;
    }

  int found = 0;
  char *line;
  while ((line = read_line(maps)))
    {
      if (strstr(line, object))
        {
          char *tmp = strchr(line, '-');
          if (tmp)
            {
              *tmp = '\0';
              sscanf(line, "%lx", (long unsigned *) base);
              found = 1;
              break;
            }
        }
    }

  fclose(maps);
  free(path);

  return found;
}

char *
proc_get_object_path(vaddr_t object_base)
{
  static char *object_path = NULL;

  if (object_path)
    {
      free(object_path);
      object_path = NULL;
    }

  char *path =
      (char *) malloc(strlen("/proc/") + PID_MAX + strlen("/maps") + 1);
  if (path == NULL)
    {
      printf("err: %s\n", strerror(errno));
      return NULL;
    }
  sprintf(path, "/proc/%u/maps", pid);
  FILE *maps = fopen(path, "r");
  if (maps == NULL)
    {
      printf("err: %s\n", strerror(errno));
      free(path);
      return NULL;
    }

  char *line;
  while ((line = read_line(maps)))
    {
      char *tmp = strchr(line, '-');
      if (tmp)
        {
          *tmp++ = '\0';
          vaddr_t base;
          sscanf(line, "%lx", (long unsigned *) &base);
          if (base == object_base)
            {
              object_path = (char *) malloc(strlen(tmp));
              strcpy(object_path, strchr(tmp, '/'));
              break;
            }
        }
    }

  fclose(maps);
  free(path);

  return object_path;
}

char *
proc_get_executable()
{
  static char *executable;

  if (executable)
    {
      free(executable);
      executable = NULL;
    }

  char *path = (char *) malloc(strlen("/proc/") + PID_MAX + strlen("/exe") + 1);
  if (path == NULL)
    {
      printf("err: %s\n", strerror(errno));
      return NULL;
    }
  sprintf(path, "/proc/%u/exe", pid);
  executable = (char *) malloc(PATH_MAX); // let's hope that one is defined :(
  if (readlink(path, executable, PATH_MAX) == -1)
    {
      printf("err: %s\n", strerror(errno));
      free(path);
      return NULL;
    }

  free(path);

  return executable;
}

int
proc_iterate_addrspace(vaddr_t *base)
{
  static FILE *maps = NULL;
  if (maps == NULL)
    {
      char *path = (char *) malloc(strlen("/proc/") + PID_MAX + strlen("/maps")
          + 1);
      if (path == NULL)
        {
          printf("err: %s\n", strerror(errno));
          return 0;
        }
      sprintf(path, "/proc/%u/maps", pid);
      maps = fopen(path, "r");
      if (maps == NULL)
        {
          printf("err: %s\n", strerror(errno));
          free(path);
          return 0;
        }
      free(path);
    }

  char *line;
  while ((line = read_line(maps)))
    {
      char *tmp = strchr(line, '-');
      if (tmp)
        {
          *tmp++ = '\0';
          strtok(tmp, " ");
          tmp = strtok(NULL, " ");
          if (strchr(tmp, 'r'))
            {
              strtok(NULL, " ");
              strtok(NULL, " ");
              strtok(NULL, " ");
              tmp = strtok(NULL, " ");
              if (tmp)
                {
                  if (strstr(tmp, "/dev") == tmp)
                    {
                      continue;
                    }
                }
              sscanf(line, "%lx", (long unsigned *) base);
              return 1;
            }
          else
            {
              continue;
            }
        }
    }
  fclose(maps);
  maps = NULL;

  return 0;
}
