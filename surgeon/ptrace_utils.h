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

#ifndef PTRACE_UTILS_H_
#define PTRACE_UTILS_H_

#include <sys/user.h>
#include <string.h>
#include <unistd.h>

#include "types.h"

extern pid_t pid;

#define ALIGN_SIZE(size) ((size) + (sizeof(long) -  (((size) % sizeof(long)) ? ((size) % sizeof(long)) : sizeof(long))))

long *
ptrace_read(vaddr_t, size_t);

int
ptrace_write(vaddr_t, void *, size_t);

void *
ptrace_call(vaddr_t, struct user_regs_struct *);

#endif /* PTRACE_UTILS_H_ */
