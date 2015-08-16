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

#include <dlfcn.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <unistd.h>

#include "elf_utils.h"
#include "proc_utils.h"
#include "ptrace_utils.h"
#include "stubs.h"
#include "types.h"

pid_t pid = 0;

char *module_to_load = NULL;

char action = 'i';

char *libc_path = NULL;

Elf32_Off malloc_off = 0;
Elf32_Off free_off = 0;
Elf32_Off dlopen_off = 0;
Elf32_Off dlclose_off = 0;

int
resolve_symbol_generic(vaddr_t base, const char *symbol, vaddr_t *resolved)
{
  if (elf_is_valid(base))
    {
      Elf32_Ehdr ehdr;
      if (elf_read_ehdr(base, ehdr))
        {
          if (ehdr.e_type == ET_DYN)
            {
              vaddr_t phdr_table = base + ehdr.e_phoff;
              int i;
              for (i = 0; i < ehdr.e_phnum * ehdr.e_phentsize; i
                  += ehdr.e_phentsize)
                {
                  Elf32_Phdr phdr;
                  if (elf_read_phdr(phdr_table + i, phdr))
                    {
                      if (phdr.p_type == PT_DYNAMIC)
                        {
                          vaddr_t dyn_section = base + phdr.p_offset;
                          Elf32_Off dyn_off = 0;
                          vaddr_t symtab = 0, strtab = 0;
                          long nchain = 0;
                          Elf32_Dyn dyn;
                          do
                            {
                              if (!elf_read_dyn(dyn_section + dyn_off, dyn))
                                {
                                  break;
                                }
                              dyn_off += sizeof(Elf32_Dyn);
                              if (dyn.d_tag == DT_SYMTAB)
                                {
                                  symtab
                                      = ((vaddr_t) dyn.d_un.d_ptr < base) ? base
                                          + dyn.d_un.d_ptr
                                          : (vaddr_t) dyn.d_un.d_ptr;
                                }
                              else if (dyn.d_tag == DT_STRTAB)
                                {
                                  strtab
                                      = ((vaddr_t) dyn.d_un.d_ptr < base) ? base
                                          + dyn.d_un.d_ptr
                                          : (vaddr_t) dyn.d_un.d_ptr;
                                }
                              else if (dyn.d_tag == DT_HASH)
                                {
                                  vaddr_t hash = ((vaddr_t) dyn.d_un.d_ptr
                                      < base) ? base + dyn.d_un.d_ptr
                                      : (vaddr_t) dyn.d_un.d_ptr;
                                  if (!elf_read(hash + sizeof(Elf32_Word),
                                      &nchain, sizeof(Elf32_Word)))
                                    {
                                      break;
                                    }
                                }
                            }
                          while (dyn.d_tag != DT_NULL && !(symtab && strtab
                              && nchain));
                          if (symtab && strtab && nchain)
                            {
                              int j;
                              for (j = 0; j < nchain * sizeof(Elf32_Sym); j
                                  += sizeof(Elf32_Sym))
                                {
                                  Elf32_Sym sym;
                                  if (elf_read_sym(symtab + j, sym))
                                    {
                                      if (sym.st_name != 0)
                                        {
                                          char *sym_name = elf_read_string(
                                              strtab + sym.st_name);
                                          if (sym_name)
                                            {
                                              if (strcmp(symbol, sym_name) == 0)
                                                {
                                                  *resolved
                                                      = ((vaddr_t) sym.st_value
                                                          < base) ? base
                                                          + sym.st_value
                                                          : (vaddr_t) sym.st_value;
                                                  return 1;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
  return 0;
}

int
resolve_symbol_dumb(const char *object, const char *symbol, vaddr_t *resolved)
{
  char *cmd = (char *) malloc(strlen("readelf -s ") + strlen(object) + strlen(
      " | grep ") + strlen(symbol) + 1);
  if (cmd == NULL)
    {
      printf("err: %s\n", strerror(errno));
      return 0;
    }
  sprintf(cmd, "readelf -s %s | grep %s", object, symbol);
  FILE *output = popen(cmd, "r");
  if (output == NULL)
    {
      printf("err: %s\n", strerror(errno));
      free(cmd);
      return 0;
    }
  char *line;
  while ((line = read_line(output)))
    {
      if (strstr(line, "GLOBAL"))
        {
          Elf32_Off off;
          strtok(line, ":");
          char *tmp = strtok(NULL, " ");
          if (tmp)
            {
              sscanf(tmp, "%x", &off);
              strtok(NULL, " ");
              strtok(NULL, " ");
              strtok(NULL, " ");
              strtok(NULL, " ");
              strtok(NULL, " ");
              char *sym = strtok(NULL, "@");
              if (sym)
                {
                  if (strcmp(symbol, sym) == 0)
                    {
                      vaddr_t base;
                      if (proc_get_object_base(object, &base))
                        {
                          *resolved = base + off;
                          pclose(output);
                          free(cmd);
                          return 1;
                        }
                    }
                }
            }
        }
    }
  pclose(output);
  free(cmd);
  return 0;
}

int
load_module(const char *module, vaddr_t patch, vaddr_t sym__libc_dlopen_mode,
    vaddr_t sym_malloc, vaddr_t sym_free)
{
  long success = 0;

  struct user_regs_struct regs_backup;
  struct user_regs_struct regs;

  if (ptrace(PTRACE_GETREGS, pid, NULL, &regs_backup) == -1)
    {
      printf("err: %s\n", strerror(errno));
      return 0;
    }
  memcpy(&regs, &regs_backup, sizeof(struct user_regs_struct));

  long *patch_backup;
  size_t patch_size = ALIGN_SIZE(call_malloc_STUB_size)
      + ALIGN_SIZE(call_free_STUB_size)
      + ALIGN_SIZE(call_libc_dlopen_mode_STUB_size);
  if ((patch_backup = ptrace_read(patch, patch_size)) == NULL)
    {
      printf("err: could not backup original code\n");
      return 0;
    }
  vaddr_t free_stub = patch + ALIGN_SIZE(call_malloc_STUB_size);
  vaddr_t libc_dlopen_mode_stub = free_stub + ALIGN_SIZE(call_free_STUB_size);
  if (!ptrace_write(patch, call_malloc_STUB, call_malloc_STUB_size))
    {
      printf("err: could not patch in malloc stub\n");
      return 0;
    }
  if (!ptrace_write(free_stub, call_free_STUB, call_free_STUB_size))
    {
      printf("err: could not patch in free stub\n");
      goto backup;
    }
  if (!ptrace_write(libc_dlopen_mode_stub, call_libc_dlopen_mode_STUB,
      call_libc_dlopen_mode_STUB_size))
    {
      printf("err: could not patch in __libc_dlopen_mode stub\n");
      goto backup;
    }

  regs.eax = (long) sym_malloc;
  regs.ecx = (long) ALIGN_SIZE(strlen(module) + 1);
  vaddr_t module_string = ptrace_call(patch, &regs);
  if (module_string == NULL)
    {
      printf("err: allocating memory for module name failed\n");
      goto backup;
    }
  if (!ptrace_write(module_string, (void *) module, strlen(module) + 1))
    {
      printf("err: failed to write module name\n");
      goto cleanup;
    }

  regs.eax = (long) sym__libc_dlopen_mode;
  regs.ecx = (long) RTLD_LAZY | 0x80000000;
  regs.edx = (long) module_string;
  success = (long) ptrace_call(libc_dlopen_mode_stub, &regs);

  cleanup: regs.eax = (long) sym_free;
  regs.ecx = (long) module_string;
  ptrace_call(free_stub, &regs);

  backup: if (!ptrace_write(patch, patch_backup, patch_size))
    {
      printf("err: failed to restore original code\n");
      success = 0;
    }
  if (ptrace(PTRACE_SETREGS, pid, NULL, &regs_backup) == -1)
    {
      printf("err: %s\n", strerror(errno));
      return 0;
    }

  return success;
}

int
unload_module(const char *module, vaddr_t patch, vaddr_t sym__libc_dlclose,
    vaddr_t sym__libc_dlopen_mode, vaddr_t sym_malloc, vaddr_t sym_free)
{
  struct user_regs_struct regs_backup;
  struct user_regs_struct regs;

  if (ptrace(PTRACE_GETREGS, pid, NULL, &regs_backup) == -1)
    {
      printf("err: %s\n", strerror(errno));
      return 0;
    }
  memcpy(&regs, &regs_backup, sizeof(struct user_regs_struct));

  long *patch_backup;
  size_t patch_size = ALIGN_SIZE(call_malloc_STUB_size)
      + ALIGN_SIZE(call_free_STUB_size)
      + ALIGN_SIZE(call_libc_dlopen_mode_STUB_size)
      + ALIGN_SIZE(call_libc_dlclose_STUB_size);
  if ((patch_backup = ptrace_read(patch, patch_size)) == NULL)
    {
      printf("err: could not backup original code\n");
      return 0;
    }
  vaddr_t free_stub = patch + ALIGN_SIZE(call_malloc_STUB_size);
  vaddr_t libc_dlopen_mode_stub = free_stub + ALIGN_SIZE(call_free_STUB_size);
  vaddr_t libc_dlclose_stub = libc_dlopen_mode_stub
      + ALIGN_SIZE(call_libc_dlopen_mode_STUB_size);
  if (!ptrace_write(patch, call_malloc_STUB, call_malloc_STUB_size))
    {
      printf("err: could not patch in malloc stub\n");
      return 0;
    }
  if (!ptrace_write(free_stub, call_free_STUB, call_free_STUB_size))
    {
      printf("err: could not patch in free stub\n");
      goto backup;
    }
  if (!ptrace_write(libc_dlopen_mode_stub, call_libc_dlopen_mode_STUB,
      call_libc_dlopen_mode_STUB_size))
    {
      printf("err: could not patch in __libc_dlopen_mode stub\n");
      goto backup;
    }
  if (!ptrace_write(libc_dlclose_stub, call_libc_dlclose_STUB,
      call_libc_dlclose_STUB_size))
    {
      printf("err: could not patch in __libc_dlclose stub\n");
      goto backup;
    }

  regs.eax = (long) sym_malloc;
  regs.ecx = (long) ALIGN_SIZE(strlen(module) + 1);
  vaddr_t module_string = ptrace_call(patch, &regs);
  if (module_string == NULL)
    {
      printf("err: could not allocate memory for module name\n");
      goto backup;
    }
  if (!ptrace_write(module_string, (void *) module, strlen(module) + 1))
    {
      printf("err: failed to write module name\n");
      goto cleanup;
    }

  regs.eax = (long) sym__libc_dlopen_mode;
  regs.ecx = (long) RTLD_LAZY | 0x80000000;
  regs.edx = (long) module_string;
  void *handle = ptrace_call(libc_dlopen_mode_stub, &regs);
  if (handle == NULL)
    {
      printf("err: could not get a handle for %s\n", module);
      goto cleanup;
    }

  long success = 1;
  vaddr_t base;
  while (proc_get_object_base(module, &base))
    {
      regs.eax = (long) sym__libc_dlclose;
      regs.ecx = (long) handle;
      if (ptrace_call(libc_dlclose_stub, &regs))
        {
          printf("err: __libc_dlclose returned with error\n");
          success = 0;
          break;
        }
    }

  cleanup: regs.eax = (long) sym_free;
  regs.ecx = (long) module_string;
  ptrace_call(free_stub, &regs);

  backup: if (!ptrace_write(patch, patch_backup, patch_size))
    {
      success = 0;
    }
  if (ptrace(PTRACE_SETREGS, pid, NULL, &regs_backup) == -1)
    {
      printf("err: %s\n", strerror(errno));
      return 0;
    }

  return success;
}

int
parse_arguments(int argc, char **argv)
{
  struct option long_options[] =
    {
      { "libc", required_argument, 0, 'a' },
      { "malloc", required_argument, 0, 'b' },
      { "free", required_argument, 0, 'c' },
      { "dlopen", required_argument, 0, 'd' },
      { "dlclose", required_argument, 0, 'e' },
      { 0 } };
  while (optind < argc)
    {
      int index = -1;
      int result = getopt_long(argc, argv, "hiu", long_options, &index);
      if (result == -1)
        {
          break;
        }
      switch (result)
        {
      case 'a':
        libc_path = (char *) malloc(strlen(optarg) + 1);
        if (libc_path == NULL)
          {
            printf("err: %s\n", strerror(errno));
            return 0;
          }
        strcpy(libc_path, optarg);
        break;
      case 'b':
        if (!sscanf(optarg, "%lx", (long unsigned *) &malloc_off))
          {
            return 0;
          }
        break;
      case 'c':
        if (!sscanf(optarg, "%lx", (long unsigned *) &free_off))
          {
            return 0;
          }
        break;
      case 'd':
        if (!sscanf(optarg, "%lx", (long unsigned *) &dlopen_off))
          {
            return 0;
          }
        break;
      case 'e':
        if (!sscanf(optarg, "%lx", (long unsigned *) &dlclose_off))
          {
            return 0;
          }
        break;
      case 'h':
        return 0;
      case 'i':
      case 'u':
        action = result;
        break;
      case '?':
        printf("err: unknown parameter\n");
        return 0;
      case ':':
        printf("err: missing argument\n");
        return 0;
        }
    }
  if (argc - optind < 2)
    {
      return 0;
    }
  if (!sscanf(argv[optind++], "%i", &pid))
    {
      return 0;
    }
  module_to_load = (char *) malloc(strlen(argv[optind]) + 1);
  strcpy(module_to_load, argv[optind++]);
  return 1;
}

void
usage()
{
  printf("\n");
  printf("usage:\n");
  printf("      surgeon [OPTION]... PID MODULE\n");
  printf("\n");
  printf("      OPTIONS:\n");
  printf("        -h                display usage\n");
  printf("        -i                inject MODULE into process PID (default)\n");
  printf("        -u                unload MODULE from process PID\n");
  printf("        --libc=PATH       set location of libc to PATH\n");
  printf("        --malloc=OFFSET   set offset for symbol malloc to OFFSET\n");
  printf("        --free=OFFSET     set offset for symbol free to OFFSET\n");
  printf(
      "        --dlopen=OFFSET   set offset for symbol __libc_dlopen_mode to OFFSET\n");
  printf(
      "        --dlclose=OFFSET  set offset for symbol __libc_dlclose to OFFSET\n");
  printf("\n");
}

int
main(int argc, char **argv)
{
  if (!parse_arguments(argc, argv))
    {
      usage();
      exit(EXIT_FAILURE);
    }

  if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1)
    {
      printf("err: could not attach to process %i (%s)\n", pid, strerror(errno));
      exit(EXIT_FAILURE);
    }

  vaddr_t libc_base = 0, patch = 0, p_malloc = 0, p_free = 0, p_dlopen = 0,
      p_dlclose = 0;

  if (libc_path)
    {
      proc_get_object_base(libc_path, &libc_base);
    }
  else
    {
      proc_get_object_base("libc-", &libc_base);
    }

  if (libc_base)
    {
      if (libc_path == NULL)
        {
          libc_path = proc_get_object_path(libc_base);
        }
      if (malloc_off)
        {
          p_malloc = libc_base + malloc_off;
        }
      else if (!resolve_symbol_generic(libc_base, "malloc", &p_malloc))
        {
          if (libc_path)
            {
              resolve_symbol_dumb(libc_path, "malloc", &p_malloc);
            }
        }
      if (free_off)
        {
          p_free = libc_base + free_off;
        }
      else if (!resolve_symbol_generic(libc_base, "free", &p_free))
        {
          if (libc_path)
            {
              resolve_symbol_dumb(libc_path, "free", &p_free);
            }
        }
      if (dlopen_off)
        {
          p_dlopen = libc_base + dlopen_off;
        }
      else if (!resolve_symbol_generic(libc_base, "__libc_dlopen_mode",
          &p_dlopen))
        {
          if (libc_path)
            {
              resolve_symbol_dumb(libc_path, "__libc_dlopen_mode", &p_dlopen);
            }
        }
      if (dlclose_off && action == 'u')
        {
          p_dlclose = libc_base + dlclose_off;
        }
      else if (!resolve_symbol_generic(libc_base, "__libc_dlclose", &p_dlclose)
          && action == 'u')
        {
          if (libc_path)
            {
              resolve_symbol_dumb(libc_path, "__libc_dlclose", &p_dlclose);
            }
        }
    }
  else
    {
      vaddr_t addr;
      while (proc_iterate_addrspace(&addr))
        {
          if (!p_malloc)
            {
              resolve_symbol_generic(addr, "malloc", &p_malloc);
            }
          if (!p_free)
            {
              resolve_symbol_generic(addr, "free", &p_free);
            }
          if (!p_dlopen)
            {
              resolve_symbol_generic(addr, "__libc_dlopen_mode", &p_dlopen);
            }
          if (action == 'u' && !p_dlclose)
            {
              resolve_symbol_generic(addr, "__libc_dlclose", &p_dlclose);
            }
        }
    }

  char *bin = proc_get_executable();
  if (bin)
    {
      proc_get_object_base(bin, &patch);
    }

  if (!(patch && p_malloc && p_free && p_dlopen && (action == 'i' || p_dlclose)))
    {
      printf("err: could not resolve all necessary symbols\n");
      ptrace(PTRACE_DETACH, pid, NULL, NULL);
      exit(EXIT_FAILURE);
    }
  else
    {
      int success;
      switch (action)
        {
      case 'i':
        printf("injecting module %s into process %i\n", module_to_load, pid);
        success
            = load_module(module_to_load, patch, p_dlopen, p_malloc, p_free);
        if (!success)
          {
            printf("err: failed to load module %s\n", module_to_load);
          }
        else
          {
            printf("successfully injected module %s\n", module_to_load);
          }
        break;
      case 'u':
        printf("unloading module %s from process %i\n", module_to_load, pid);
        success = unload_module(module_to_load, patch, p_dlclose, p_dlopen,
            p_malloc, p_free);
        if (!success)
          {
            printf("err: failed to unload module %s\n", module_to_load);
          }
        else
          {
            printf("successfully unloaded module %s\n", module_to_load);
          }
        break;
        }
      if (!success)
        {
          ptrace(PTRACE_DETACH, pid, NULL, NULL);
          exit(EXIT_FAILURE);
        }
    }

  ptrace(PTRACE_DETACH, pid, NULL, NULL);
  exit(EXIT_SUCCESS);
}
