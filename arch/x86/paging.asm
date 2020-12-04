global paging_enable

; extern void paging_enable(uint32_t page_directory);
paging_enable:
	push ebp
	mov ebp, esp
	mov eax, [ebp + 8]
	mov cr3, eax
	mov eax, cr0
	or eax, 0x80000001 ; set paging bit in cr0
	mov cr0, eax
	mov esp, ebp
	pop ebp
	ret