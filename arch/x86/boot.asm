;https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

;declare constant for the multiboot header
MBALIGN		equ 1 << 0 ;aligns modules on 4Kb pages boundaries
MEMINFO		equ 1 << 1 ;if mem_* is present, include it
FLAGS		equ MBALIGN  | MEMINFO 
MAGIC		equ 0x1BADB002
CHECKSUM	equ -(MAGIC + FLAGS)

;declare a multiboot header that marks the program as a kernel
section .multiboot_data
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM

;declare a stack into bss section
section .bss

align 4096
global page_directory
page_directory:
	resb 4096

align 4096
page_table:
	resb 4096

; align 4096
; page_table_grub:
; 	resb 4096

align 4096
stack_bottom:
	resb 16384
align 4096
stack_top:


section .multiboot_text
	global _start:function

	extern _kernel_start
	extern _kernel_end

_start:
	; phy addr of page table
	mov edi, page_table
	sub edi, 0xC0000000
	; start at first addr
	mov esi, 0
	; page table size
	mov ecx, 1024

virt_kernel_map_start:
	; set attributes
	; - present, writtable 0b..011
	; and put the addr aligned on 4096 in page table
	mov edx, esi
	or edx, 0x03
	mov dword [edi], edx
	; increment base addr of next page of 4096
	add esi, 4096
	; increment phy addr of page table of 4 bytes (size of page table entry)
	add edi, 4
	; use loop to dec ecx
	loop virt_kernel_map_start

; 	mov edi, page_table_grub
; 	sub edi, 0xC0000000
; 	mov esi, ebx
; 	mov ecx, 1024

; virt_grub_info_map_start
; 	mov edx, esi
; 	or edx, 0x03
; 	mov dword [edi], edx
; 	add esi, 4096
; 	add edi, 4
; 	loop virt_grub_info_map_start

	; first entry of page directory is our kernel page table 
	; we started at 0x0, so identity mapping here
	; prevent error as CPU next instruction after paging enables is fetched
	; directly at phys addr. Prevent page fault
	; entry 768, 3GB higher half of memory take our page as well
	; kernel space start there from now on
	mov ecx, page_directory
	sub ecx, 0xC0000000
	mov dword [ecx], page_table - 0xC0000000 + 0x03
	mov dword [ecx + 768 * 4], page_table - 0xC0000000 + 0x03

	;mov dword [ecx + 769 * 4], page_table_grub - 0xC0000000 + 0x03

	; indicate to mmu through cr3 where to find physical addr of page directory

	mov cr3, ecx

	; enable paging in cr0 config bits
	mov ecx, cr0
	or ecx, 0x80000001
	mov cr0, ecx

	; absolute jmp
	lea ecx, [virt_kernel_map_end]
	jmp ecx

section .text
virt_kernel_map_end:
	;unmap identity mapping
	mov dword [page_directory], 0

	;TLB flush
	mov ecx, cr3
	mov cr3, ecx

	;setup kernel stack
	mov esp, stack_top
	xor ebp, ebp

	push ebx
	push eax

	extern kmain
	call kmain

	;clear interupt flags, processor
	;won't handle maskable interupts anymore
	cli

.hang:
	hlt
	jmp .hang

.end:
