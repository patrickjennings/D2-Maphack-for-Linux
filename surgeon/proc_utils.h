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

#ifndef PROC_UTILS_H_
#define PROC_UTILS_H_

#include <stdio.h>
#include <unistd.h>

#include "types.h"

extern pid_t pid;

char *
read_line(FILE *);

int
proc_get_object_base(const char *, vaddr_t *);

char *
proc_get_object_path(vaddr_t);

char *
proc_get_executable();

int
proc_iterate_addrspace(vaddr_t *);

#endif /* PROC_UTILS_H_ */
