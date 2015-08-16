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

#ifndef ELF_UTILS_H_
#define ELF_UTILS_H_

#include <linux/elf.h>
#include <string.h>

#include "types.h"

extern pid_t pid;

int
elf_is_valid(vaddr_t);

int
elf_read(vaddr_t, void *, size_t);

#define elf_read_ehdr(addr, ehdr) elf_read(addr, &ehdr, sizeof(Elf32_Ehdr))

#define elf_read_phdr(addr, phdr) elf_read(addr, &phdr, sizeof(Elf32_Phdr))

#define elf_read_shdr(addr, shdr) elf_read(addr, &shdr, sizeof(Elf32_Shdr))

#define elf_read_dyn(addr, dyn) elf_read(addr, &dyn, sizeof(Elf32_Dyn))

#define elf_read_sym(addr, sym) elf_read(addr, &sym, sizeof(Elf32_Sym))

char *
elf_read_string(vaddr_t);

void
elf_dump_ehdr(Elf32_Ehdr);

void
elf_dump_phdr(Elf32_Phdr);

void
elf_dump_shdr(Elf32_Shdr);

void
elf_dump_dyn(Elf32_Dyn);

void
elf_dump_sym(Elf32_Sym);

#endif /* ELF_UTILS_H_ */
