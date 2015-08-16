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
#include <sys/ptrace.h>
#include <unistd.h>

#include "elf_utils.h"
#include "ptrace_utils.h"

int
elf_is_valid(vaddr_t addr)
{
  char *word = (char *) ptrace_read(addr, sizeof(long));
  if (word == NULL)
    {
      return 0;
    }
  return (word[EI_MAG0] == ELFMAG0 && word[EI_MAG1] == ELFMAG1 && word[EI_MAG2]
      == ELFMAG2 && word[EI_MAG3] == ELFMAG3) ? 1 : 0;
}

int
elf_read(vaddr_t addr, void *elf_struct, size_t size)
{
  long *data = ptrace_read(addr, size);
  if (data)
    {
      memcpy(elf_struct, (void *) data, size);
      return 1;
    }
  return 0;
}

char *
elf_read_string(vaddr_t addr)
{
  static char *str = NULL;

  if (str)
    {
      free(str);
      str = NULL;
    }

  int i = 0;
  do
    {
      if ((i % 512) == 0)
        {
          str = (char *) realloc(str, i + 512);
          if (str == NULL)
            {
              printf("err: %s\n", strerror(errno));
              return NULL;
            }
        }

      long *word = ptrace_read(addr + i, sizeof(long));
      if (word == NULL)
        {
          return NULL;
        }
      memcpy(str + i, word, sizeof(long));
    }
  while (str[i++] != '\0' && str[i++] != '\0' && str[i++] != '\0' && str[i++]
      != '\0');

  return str;
}

void
elf_dump_ehdr(Elf32_Ehdr ehdr)
{
  printf("-----------------------------------------------------\n");
  printf("ELF header: (magic sequence %lX %lX %lX %lX)\n",
      (long unsigned) ehdr.e_ident[EI_MAG0],
      (long unsigned) ehdr.e_ident[EI_MAG1],
      (long unsigned) ehdr.e_ident[EI_MAG2],
      (long unsigned) ehdr.e_ident[EI_MAG3]);
  printf("  class:                         ");
  switch (ehdr.e_ident[EI_CLASS])
    {
  case ELFCLASSNONE:
    printf("invalid\n");
    break;
  case ELFCLASS32:
    printf("ELF32\n");
    break;
  case ELFCLASS64:
    printf("ELF64\n");
    break;
  default:
    printf("\n");
    break;
    }
  printf("  data:                          ");
  switch (ehdr.e_ident[EI_DATA])
    {
  case ELFDATANONE:
    printf("invalid\n");
    break;
  case ELFDATA2LSB:
    printf("2's complement (LSB)\n");
    break;
  case ELFDATA2MSB:
    printf("2's complement (MSB)\n");
    break;
  default:
    printf("\n");
    break;
    }
  printf("  version:                       %i\n", ehdr.e_ident[EI_VERSION]);
  printf("\n");
  printf("  type:                          ");
  switch (ehdr.e_type)
    {
  case ET_NONE:
    printf("invalid\n");
    break;
  case ET_REL:
    printf("relocatable file\n");
    break;
  case ET_EXEC:
    printf("executable file\n");
    break;
  case ET_DYN:
    printf("shared object file\n");
    break;
  case ET_CORE:
    printf("core file\n");
    break;
  default:
    printf("\n");
    break;
    }
  printf("  machine:                       ");
  switch (ehdr.e_machine)
    {
  case EM_NONE:
    printf("invalid\n");
    break;
  case EM_M32:
    printf("AT&T WE 32100\n");
    break;
  case EM_SPARC:
    printf("SPARC\n");
    break;
  case EM_386:
    printf("Intel 80386\n");
    break;
  case EM_68K:
    printf("Motorola 68000\n");
    break;
  case EM_88K:
    printf("Motorola 88000\n");
    break;
  case EM_860:
    printf("Intel 80860\n");
    break;
  case EM_MIPS:
    printf("MIPS RS3000\n");
    break;
  default:
    printf("\n");
    break;
    }
  printf("  flags:                         0x%lX\n",
      (long unsigned) ehdr.e_flags);
  printf("\n");
  printf("  entry point:                   0x%lX\n",
      (long unsigned) ehdr.e_entry);
  printf("\n");
  printf("  program header table (offset): %i bytes\n", ehdr.e_phoff);
  printf("  size of entry:                 %i bytes\n", ehdr.e_phentsize);
  printf("  number of entries:             %i\n", ehdr.e_phnum);
  printf("\n");
  printf("  section header table (offset): %i bytes\n", ehdr.e_shoff);
  printf("  size of entry:                 %i bytes\n", ehdr.e_shentsize);
  printf("  number of entries:             %i\n", ehdr.e_shnum);
  printf("  index of string table:         %i\n", ehdr.e_shstrndx);
  printf("\n");
  printf("  size of ELF header:            %i bytes\n", ehdr.e_ehsize);
  printf("-----------------------------------------------------\n");
  printf("\n");
}

void
elf_dump_phdr(Elf32_Phdr phdr)
{
  printf("-----------------------------------------------------\n");
  printf("program header:\n");
  printf("  type:                ");
  switch (phdr.p_type)
    {
  case PT_NULL:
    printf("unused\n");
    break;
  case PT_LOAD:
    printf("loadable segment\n");
    break;
  case PT_DYNAMIC:
    printf("dynamic linking information\n");
    break;
  case PT_INTERP:
    printf("interpreter information\n");
    break;
  case PT_NOTE:
    printf("auxiliary information\n");
    break;
  case PT_SHLIB:
    printf("warning: segment type is reserved but has unspeciﬁed semantics\n");
    break;
  case PT_PHDR:
    printf("program header table information\n");
    break;
  default:
    printf("\n");
    break;
    }
  printf("  offset:              0x%lX\n", (long unsigned) phdr.p_offset);
  printf("  virtual address:     0x%lX\n", (long unsigned) phdr.p_vaddr);
  printf("  physical address:    0x%lX\n", (long unsigned) phdr.p_paddr);
  printf("  size (file image):   %i\n", phdr.p_filesz);
  printf("  size (memory image): %i\n", phdr.p_memsz);
  printf("  flags:               ");
  printf((phdr.p_flags & PF_R) ? "R" : " ");
  printf((phdr.p_flags & PF_W) ? "W" : " ");
  printf((phdr.p_flags & PF_X) ? "X" : " ");
  printf("\n");
  printf("  alignment:           %i\n", phdr.p_align);
  printf("-----------------------------------------------------\n");
  printf("\n");
}

void
elf_dump_shdr(Elf32_Shdr shdr)
{
  printf("-----------------------------------------------------\n");
  printf("section header:\n");
  printf("  name:         0x%lX\n", (long unsigned) shdr.sh_name);
  printf("  type:         ");
  switch (shdr.sh_type)
    {
  case SHT_NULL:
    printf("inactive\n");
    break;
  case SHT_PROGBITS:
    printf("information defined by program\n");
    break;
  case SHT_SYMTAB:
    printf("symbol table (SYMTAB)\n");
    break;
  case SHT_STRTAB:
    printf("string table\n");
    break;
  case SHT_RELA:
    printf("relocation entries (explicit addends)\n");
    break;
  case SHT_HASH:
    printf("symbol hash table\n");
    break;
  case SHT_DYNAMIC:
    printf("information for dynamic linking\n");
    break;
  case SHT_NOTE:
    printf("note section\n");
    break;
  case SHT_NOBITS:
    printf("section occupies no space\n");
    break;
  case SHT_REL:
    printf("relocation entries\n");
    break;
  case SHT_SHLIB:
    printf("reserved but unspecified semantics\n");
    break;
  case SHT_DYNSYM:
    printf("symbol table (DYNSYM)\n");
    break;
  default:
    printf("\n");
    break;
    }
  printf("  flags:        ");
  printf((shdr.sh_flags & SHF_WRITE) ? "W" : " ");
  printf((shdr.sh_flags & SHF_ALLOC) ? "A" : " ");
  printf((shdr.sh_flags & SHF_EXECINSTR) ? "X" : " ");
  printf("\n");
  printf("  address:      0x%lX\n", (long unsigned) shdr.sh_addr);
  printf("  offset:       %i bytes\n", shdr.sh_offset);
  printf("  size:         %i bytes\n", shdr.sh_size);
  printf("  link:         0x%lX\n", (long unsigned) shdr.sh_link);
  printf("  info:         0x%lX\n", (long unsigned) shdr.sh_info);
  printf("  alignment:    %i\n", shdr.sh_addralign);
  printf("  size (entry): %i bytes\n", shdr.sh_entsize);
  printf("-----------------------------------------------------\n");
  printf("\n");
}

void
elf_dump_dyn(Elf32_Dyn dyn)
{
  printf("-----------------------------------------------------\n");
  printf("dynamic structure:\n");
  printf("  tag:             ");
  switch (dyn.d_tag)
    {
  case DT_NULL:
    printf("end of _DYNAMIC array\n");
    break;
  case DT_NEEDED:
    printf("needed library\n");
    break;
  case DT_PLTRELSZ:
    printf("size of relocation entries\n");
    break;
  case DT_PLTGOT:
    printf("procedure linkage table / global offset table\n");
    break;
  case DT_HASH:
    printf("symbol hash table\n");
    break;
  case DT_STRTAB:
    printf("string table\n");
    break;
  case DT_SYMTAB:
    printf("symbol table\n");
    break;
  case DT_RELA:
    printf("relocation table\n");
    break;
  case DT_RELASZ:
    printf("size of relocation table\n");
    break;
  case DT_RELAENT:
    printf("size of relocation table entry\n");
    break;
  case DT_STRSZ:
    printf("size of string table\n");
    break;
  case DT_SYMENT:
    printf("size of symbol table entry\n");
    break;
  case DT_INIT:
    printf("initialization function\n");
    break;
  case DT_FINI:
    printf("termination function\n");
    break;
  case DT_SONAME:
    printf("shared object\n");
    break;
  case DT_RPATH:
    printf("library search path\n");
    break;
  case DT_SYMBOLIC:
    printf("alters symbol resolution algorithm\n");
    break;
  case DT_REL:
    printf("relocation table (implicit addends)\n");
    break;
  case DT_RELSZ:
    printf("size of relocation table (implicit addends)\n");
    break;
  case DT_RELENT:
    printf("size of relocation table (implicit adddends) entry\n");
    break;
  case DT_PLTREL:
    printf("type of relocation entry (procedure linkage table)\n");
    break;
  case DT_DEBUG:
    printf("debugging\n");
    break;
  case DT_TEXTREL:
    printf("no modification to non-writable segments\n");
    break;
  case DT_JMPREL:
    printf("address of relocation entry\n");
    break;
  default:
    printf("\n");
    break;
    }
  printf("  value:           0x%lX\n", (long unsigned) dyn.d_un.d_val);
  printf("-----------------------------------------------------\n");
  printf("\n");
}

void
elf_dump_sym(Elf32_Sym sym)
{
  printf("-----------------------------------------------------\n");
  printf("symbol table entry:\n");
  printf("  name:                       0x%lX\n", (long unsigned) sym.st_name);
  printf("  value:                      0x%lX\n", (long unsigned) sym.st_value);
  printf("  size:                       %i bytes\n", sym.st_size);
  printf("  info:                       %i\n", sym.st_info);
  printf("  other:                      %i\n", sym.st_other);
  printf("  section header table index: 0x%lX\n", (long unsigned) sym.st_shndx);
  printf("-----------------------------------------------------\n");
  printf("\n");
}
