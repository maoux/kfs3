#include <kfs/kernel.h>
#include <kfs/pages.h>
#include <kfs/mem.h>

/*
	Critical kernel part
	Should be executed without error after GDT setup

	mm stand for memory management
*/
extern int		mm_init(void *page_directory_vaddr)
{
	if (pmm_init() == 0) {
		printk(KERN_INFO "Physical Memory Manager Setup done\n");
	} else {
		printk(KERN_CRIT "Physical Memory Manager Setup failed\n");
		return (1);
	}

	if (vmm_init(page_directory_vaddr) == 0) {
		printk(KERN_INFO "Virtual Memory Manager Setup done\n");
	} else {
		printk(KERN_CRIT "Virtual Memory Manager Setup failed\n");
		return (1);
	}

	if (vmalloc_init() == 0) {
		printk(KERN_INFO "vmalloc Setup done\n");
	} else {
		printk(KERN_CRIT "vmalloc Setup failed\n");
		return (1);
	}

	return (0);
}