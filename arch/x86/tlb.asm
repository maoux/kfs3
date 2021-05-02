section .text
align 4

global flush_TLB
flush_TLB:
	mov		eax, [esp + 4]
	invlpg	[eax]
	ret