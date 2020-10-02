global gdt_flush
extern _GDTR

gdt_flush:
	;Protected-mode -> virtual interrupts are not enabled,
	;CLI clears the IF flag in the EFLAGS register.
	;No other flags are affected.
	;Clearing the IF flag causes the processor
	;to ignore maskable external interrupts.
	cli

	lgdt [_GDTR]

	;far jump seg_locator:offset(addr/sym)
	;0x08 => kernel code seg descriptor
	jmp 0x08:gdt_flush_complete

;Segment selector : 16bits
;	- index:			13 bits
;	- table indicator:	1 bit -> 0 GDT / 1 LDT
;	- privilege bits:	2 bits -> 00 lowest (kernel) / 11 highest (user)
gdt_flush_complete:
	;position every seg registers on kernel data seg
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	;position ss to kernel stack seg
	mov ax, 0x18
	mov ss, ax
	ret
