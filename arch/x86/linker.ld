ENTRY(_start)

SECTIONS
{
	. = 0x00100000;

	.multiboot_data : {
		*(.multiboot_data)
	}

	.multiboot_text : {
		*(.multiboot_text)
	}

	. += 0xC0000000;
	_kernel_start = .;

	.text ALIGN(4K) : AT (ADDR (.text) - 0xC0000000)
	{
		*(.text)
	}

	.rodata ALIGN(4K) : AT (ADDR (.rodata) - 0xC0000000)
	{
		*(.rodata)
	}

	.data ALIGN(4K) : AT (ADDR (.data) - 0xC0000000)
	{
		*(.data)
	}

	.bss ALIGN(4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		*(.COMMON)
		*(.bss)
	}
	.stab ALIGN(4K) : AT (ADDR (.stab) - 0xC0000000)
	{
		*(.stab)
	}
	.stabstr ALIGN(4K) : AT (ADDR (.stabstr) - 0xC0000000)
	{
		*(.stabstr)
	}
	_kernel_end = .;
}