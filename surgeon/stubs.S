
# Copyright (C) 2010 gonzoj
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
 

.section .text

.globl call_malloc_STUB
.globl call_free_STUB
.globl call_libc_dlopen_mode_STUB
.globl call_libc_dlclose_STUB

.type call_malloc_STUB, @function
call_malloc_STUB:

	push %ecx
	call *%eax
	int $0x3

.type call_free_STUB, @function
call_free_STUB:

	push %ecx
	call *%eax
	int $0x3

.type call_libc_dlopen_mode_STUB, @function
call_libc_dlopen_mode_STUB:

	push %ecx
	push %edx
	call *%eax
	int $0x3

.type call_libc_dlclose_STUB, @function
call_libc_dlclose_STUB:

	push %ecx
	call *%eax
	int $0x3

