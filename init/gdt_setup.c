#include <kfs/gdt.h>
#include <string.h>

static void		init_segment_descriptor(t_gdt_descriptor *descriptor,
								uint32_t base_addr, uint32_t limit,
								uint8_t access, uint8_t flags)
{
    descriptor->limit0_15 = (limit & 0xffff);
    descriptor->base0_15 = (base_addr & 0xffff);
    descriptor->base16_23 = (base_addr & 0xff0000) >> 16;
    descriptor->access = access;
    descriptor->limit16_19 = (limit & 0xf0000) >> 16;
    descriptor->flags = (flags & 0x0f);
    descriptor->base24_31 = (base_addr & 0xff000000) >> 24;
} 

/*
	kernel code:	access = 0x9a <=> 0b1001 1010
					flags = 0x0c <=> 1100
	kernel data:	access = 0x92 <=> 0b1001 0010
					flags = 0x0c
	kernel stack:	access = 0x96 <=> 0b1001 1010
					flags = 0x0c
	user code:		access = 0xfa <=> 0b1111 1010
					flags = 0x0c
	user data:		access = 0xf2 <=> 0b1111 0010
					flags = 0x0c
	user stack:		access = 0xf6 <=> 0b1111 1010
					flags = 0x0c
*/
extern void		init_gdt(void)
{
	t_gdt_descriptor	*gdt = (t_gdt_descriptor *)GDT_BASE_ADDR;

    init_segment_descriptor(&gdt[0], 0x0, 0x0, 0x0, 0x0);

    init_segment_descriptor(&gdt[1], 0x0, 0xffffffff, 0x9a, 0x0c);
    init_segment_descriptor(&gdt[2], 0x0, 0xffffffff, 0x92, 0x0c);
    init_segment_descriptor(&gdt[3], 0x0, 0xffffffff, 0x96, 0x0c);

    init_segment_descriptor(&gdt[4], 0x0, 0xffffffff, 0xFa, 0x0c);
    init_segment_descriptor(&gdt[5], 0x0, 0xffffffff, 0xF2, 0x0c);
    init_segment_descriptor(&gdt[6], 0x0, 0xffffffff, 0xF6, 0x0c);

    _GDTR.size = sizeof(gdt[0]) * GDT_SIZE;
    _GDTR.base_addr = GDT_BASE_ADDR;

    gdt_flush();
}