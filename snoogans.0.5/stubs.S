
#  Copyright (C) 2010 gonzoj
#
#  Please check the CREDITS file for further information.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.


.section .text

# assembly stubs for hooking

.globl draw_ingame_STUB

.globl draw_automap_STUB

.globl draw_message_log_STUB

.globl view_inventory_patch_1
.globl view_inventory_patch_2
.globl view_inventory_patch_3

.globl item_name_patch_STUB

.type draw_ingame_STUB, @function
draw_ingame_STUB:

	call draw_ingame
	pop %esi
	pop %ebx
	pop %ecx
	ret $0x4

.type draw_automap_STUB, @function
draw_automap_STUB:

	call draw_automap
	pop %edi
	pop %esi
	ret

.type draw_message_log_STUB, @function
draw_message_log_STUB:

	# call draw_message_log
	pop %edi
	pop %esi
	pop %ebp
	add $0x800, %esp
	ret $0x10

.type view_inventory_patch_1, @function
view_inventory_patch_1:

	push %eax
	call view_inventory_patch
	mov %eax, %esi
	pop %eax
	ret

.type view_inventory_patch_2, @function
view_inventory_patch_2:

	push %eax;
	call get_view_unit
	mov %eax, %ebx
	pop %eax
	ret

.type view_inventory_patch_3, @function
view_inventory_patch_3:

	push %eax
	push %ebx
	call get_view_unit
	movl (%edi), %ebx
	cmp $1, %ebx
	je old_code

	mov %eax, %edi

	old_code:
	pop %ebx
	pop %eax
	test %eax, %eax
	movl 0x60(%edi), %ecx

	ret

.type item_name_patch_STUB, @function
item_name_patch_STUB:

	mov %ebx, %ecx
	mov %edi, %edx
	call item_name_patch
	mov 0x12a(%ebp), %al
	ret
