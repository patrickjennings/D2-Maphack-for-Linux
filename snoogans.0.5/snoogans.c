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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "kernel32.h"
#include "maphack.h"
#include "patch.h"
#include "populate.h"

#define ABORT printf("\n*** FAILED TO LOAD MODULE ***\n\n"); printf("exiting...\n"); return;

void __attribute__((constructor))
_init_module()
{
  printf("\n\n*** INIT MODULE ***\n\n");
  printf("populating kernel32 functions... ");
  if (populate_kernel32_funcs())
    {
      printf("done\n");
    }
  else
    {
      ABORT
    }
  printf("populating user32 functions... ");
  if (populate_user32_funcs())
    {
      printf("done\n");
    }
  else
    {
      ABORT
    }
  printf("populating d2 functions... ");
  if (populate_d2funcs())
    {
      printf("done\n");
    }
  else
    {
      ABORT
    }
  printf("populating d2 variables... ");
  if (populate_d2vars())
    {
      printf("done\n");
    }
  else
    {
      ABORT
    }
  printf("starting maphack thread... ");
  if (pthread_create(&mh_thread, NULL, maphack, NULL))
    {
      printf("err: could not create thread\n");
      ABORT
    }
  else
    {
      printf("done\n");
    }
  printf("installing hooks... ");
  if (install_hooks())
    {
      printf("done\n");
    }
  else
    {
      ABORT
    }
  printf("\n*** MODULE LOADED ***\n\n");
}

void __attribute__((destructor))
_finit_module()
{
  printf("\n\n*** FINIT MODULE ***\n\n");
  printf("stopping maphack thread... ");
  stop_maphack();
  printf("done\n");
  printf("removing hooks... ");
  if (remove_hooks())
    {
      printf("done\n");
    }
  else
    {
      printf("*** FAILED TO UNLOAD MODULE CORRECTLY ***\n\n");
      return;
    }
  printf("\n*** MODULE UNLOADED ***\n\n");
}
