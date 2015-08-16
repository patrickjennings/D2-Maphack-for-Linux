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

#include "debug.h"
#include "kernel32.h"

#define FUNC(r, a, f, p) typedef r a (*f##_t)p; f##_t f = (f##_t) NULL; char *str_##f = #f;

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

int
manage_threads(int __attribute__((stdcall))
(*manage_thread)(void *thread))
{
  DWORD pid = GetCurrentProcessId();
  void *snap_thread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
  THREADENTRY32 thread_entry;
  DEBUG_DO(printf("managing threads... ");)
  DWORD self = GetCurrentThreadId();
  int i = 0;
  if (Thread32First(snap_thread, &thread_entry))
    {
      do
        {
          i++;
          if (thread_entry.th32OwnerProcessID != pid)
            {
              continue;
            }
          if (thread_entry.th32ThreadID == self)
            {
              continue;
            }
          void *th;
          if ((th = OpenThread(THREAD_SUSPEND_RESUME, 0,
              thread_entry.th32ThreadID)) == NULL)
            {
              printf("err: could not open thread\n");
              return 0;
            }
          if (manage_thread(th) == -1)
            {
              printf("err: could not suspend/resume thread\n");
              return 0;
            }
        }
      while (Thread32Next(snap_thread, &thread_entry));
    }
  DEBUG_DO(printf("done (%i)\n", i));
  return 1;
}
