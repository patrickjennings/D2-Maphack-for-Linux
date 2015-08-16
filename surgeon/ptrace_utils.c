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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include "ptrace_utils.h"

long *
ptrace_read(vaddr_t addr, size_t len)
{
  static long *data = NULL;

  int size = (len / sizeof(long)) + ((len % sizeof(long)) ? 1 : 0);
  if (data)
    {
      free(data);
    }
  data = (long *) malloc(size * sizeof(long));
  if (data == NULL)
    {
      printf("err: %s\n", strerror(errno));
      return NULL;
    }

  int i;
  for (i = 0; i < size; i++)
    {
      errno = 0;
      long word = ptrace(PTRACE_PEEKDATA, pid, addr + i * sizeof(long), NULL);
      if (word == -1 && errno)
        {
          printf("err: %s\n", strerror(errno));
          return NULL;
        }
      data[i] = word;
    }

  return data;
}

int
ptrace_write(vaddr_t addr, void *data, size_t len)
{
  int size = (len / sizeof(long)) + ((len % sizeof(long)) ? 1 : 0);
  int i;
  for (i = 0; i < size; i++)
    {
      long word = 0;
      size_t remaining = len - (i * sizeof(long));
      memcpy(&word, (long *) data + i, (remaining < sizeof(long)) ? remaining
          : sizeof(long));
      if (ptrace(PTRACE_POKEDATA, pid, addr + i * sizeof(long), word) == -1)
        {
          printf("err: %s\n", strerror(errno));
          return 0;
        }
    }
  return 1;
}

void *
ptrace_call(vaddr_t addr, struct user_regs_struct *regs)
{
  regs->eip = (long) addr;
  if (ptrace(PTRACE_SETREGS, pid, NULL, regs) == -1)
    {
      printf("err: %s\n", strerror(errno));
      return NULL;
    }
  if (ptrace(PTRACE_CONT, pid, NULL, NULL) == -1)
    {
      printf("err: %s\n", strerror(errno));
      return NULL;
    }
  wait(NULL);
  if (ptrace(PTRACE_GETREGS, pid, NULL, regs) == -1)
    {
      printf("err: %s\n", strerror(errno));
      return NULL;
    }
  return (void *) regs->eax;
}
