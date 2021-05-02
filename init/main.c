#include <kfs/kernel.h>
#include <kfs/multiboot.h>
#include <kfs/vga.h>
#include <kfs/keyboard.h>
#include <kfs/shell.h>
#include <kfs/gdt.h>
#include <kfs/elf.h>
#include <kfs/mem.h>
#include <kfs/pages.h>
#include <string.h>

extern void		kmain(uint32_t magic, uint32_t *meminfo_offset, void *page_directory_vaddr)
{
	uint8_t		debug = 0;

	if (magic != 0x2badb002) {
		return ;
	}

	/* Setup GDT and vga text mode array */
	video_init((uint32_t *)0xC00B8000, 80, 25);
	text_mode_intro_print();
	printk(KERN_INFO "Paging enabled\n");
	gdt_init();
	printk(KERN_INFO "GDT Setup done\n");

	/* Setup multiboot infos api and use it to setup physical memory management */
	grub_info_init((uint32_t *)((uint32_t)meminfo_offset + KERNEL_SPACE_V_ADDR));
	if (pmm_init() == 0) {
		printk(KERN_INFO "Physical Memory Manager Setup done\n");
	} else {
		printk(KERN_CRIT "Physical Memory Manager Setup failed\n");
		return ;
	}

	if (vmm_init(page_directory_vaddr) == 0) {
		printk(KERN_INFO "Virtual Memory Manager Setup done\n");
	} else {
		printk(KERN_CRIT "Virtual Memory Manager Setup failed\n");
		return ;
	}

	if (debug) {
		grub_meminfo_print();
	}

	/* keyboard test and infinite loop (break it with esc key)*/
	switch (ps2_keyboard_init()) {
		case 0:
			printk(KERN_INFO "Test PS/2 Controller passed\n");
			break ;
		case 1:
		default:
			printk(KERN_CRIT "PS/2 Controller tests failed\n");
			return ;
	}

	uint32_t	*page1 = pmm_page_get(MEM_MEDIUM);
	printk("page1: %x\n", page1);
	char		*str = (char *)0xC1000000;
	vmm_map_page((void *)page1, (void *)str, 0);
	vmm_flush_tld_entry((uint32_t)str);
	str = strcpy(str, "Hello World !\n"); 
	printk("%s\n", str);
	shell();
}