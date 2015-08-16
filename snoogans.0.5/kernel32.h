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

#ifndef KERNEL32_H_
#define KERNEL32_H_

#include "types.h"

#define TH32CS_SNAPTHREAD 0x0004

#define THREAD_SUSPEND_RESUME 0x0002

typedef struct
{
  DWORD dwSize;
  DWORD cntUsage;
  DWORD th32ThreadID;
  DWORD th32OwnerProcessID;
  LONG tpBasePri;
  LONG tpDeltaPri;
  DWORD dwFlags;
} THREADENTRY32;

#define FUNC(r, a, f, p) extern r (*f)p a; extern char *str_##f;

FUNC(void *, __attribute__((stdcall)), GetProcAddress, (void *module, int function))
FUNC(void *, __attribute__((stdcall)), GetModuleHandleA, (const char *module))
FUNC(void *, __attribute__((stdcall)), LoadLibraryA, (const char *module))
FUNC(void *, __attribute__((stdcall)), CreateToolhelp32Snapshot, (int flags, long pid))
FUNC(void *, __attribute__((stdcall)), OpenThread, (int access, int inherit_handle, int thread_id))
FUNC(int, __attribute__((stdcall)), SuspendThread, (void *thread))
FUNC(int, __attribute__((stdcall)), ResumeThread, (void *thread))
FUNC(int, __attribute__((stdcall)), Thread32First, (void *snap_thread, THREADENTRY32 *thread_entry))
FUNC(int, __attribute__((stdcall)), Thread32Next, (void *snap_thread, THREADENTRY32 *thread_entry))
FUNC(int, __attribute__((stdcall)), GetCurrentThreadId, ())
FUNC(int, __attribute__((stdcall)), GetCurrentProcessId, ())
FUNC(void *, __attribute__((stdcall)), CreateThread, (void *sa, size_t stack, void *func, void *param, int flags, void *tid))

#undef FUNC

#define _KERNEL32_FUNC_START GetProcAddress
#define _KERNEL32_FUNC_END CreateThread

#define _KERNEL32_STR_START str_GetProcAddress
#define _KERNEL32_STR_END str_CreateThread

int
manage_threads(int __attribute__((stdcall))
(*manage_thread)(void *));

#endif /* KERNEL32_H_ */
