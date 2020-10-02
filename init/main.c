#include <kfs/kernel.h>
#include <kfs/multiboot.h>
#include <kfs/vga.h>
#include <kfs/keyboard.h>
#include <kfs/shell.h>
#include <kfs/gdt.h>
#include <kfs/elf.h>

extern void		kmain(uint32_t magic, uint32_t *meminfo_offset)
{
	uint8_t		debug = 0;
	t_grub_info	*grub_info;


	if (magic != 0x2badb002) {
		return ;
	}

	/* Setup GDT */
	init_gdt();

	/* Setup multiboot infos api */
	grub_info_init(meminfo_offset);

	/* Init text mode params with multiboot specs if available */
	grub_info = (t_grub_info *)meminfo_offset;
	if (IS_GFLAG(grub_info->flags, GFLAG_FRAMEBUFFER)) {
		video_init((uint32_t *)grub_info->framebuffer_addr_low,
		grub_info->framebuffer_width, grub_info->framebuffer_height);
	}
	else {
		/*
			assume video buffer is on location 0xb8000 for now
			with standard text mode size
		*/
		video_init((uint32_t *)0xb8000, 80, 25);
	}
	text_mode_intro_print();

	if (debug) {
		grub_meminfo_print(grub_info);
	}

	/* keyboard test and infinite loop (break it with esc key)*/
	switch (ps2_keyboard_init()) {
		case 0:
			printk(KERN_INFO "Test PS/2 Controller passed\n");
			break ;
		case 1:
		default:
			printk(KERN_ERR "PS/2 Controller tests failed\n");
			return ;
	}

	shell();
}