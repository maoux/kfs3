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
	int			tmp;

	if ((tmp = pmm_init()) == 0) {
		printk(KERN_INFO "Physical Memory (Bootstrap) Manager Setup done\n");
	} else {
		printk(KERN_CRIT "Physical Memory (Bootstrap) Manager Setup failed\n");
		if (tmp == 1) {
			panic("System couldn't find memory map, boot aborted\n");
		}
		return (1);
	}

	if ((tmp = vmm_init(page_directory_vaddr)) == 0) {
		printk(KERN_INFO "Virtual Memory Manager Setup done\n");
	} else {
		printk(KERN_CRIT "Virtual Memory Manager Setup failed\n");
		return (1);
	}

	if (vmalloc_init() == 0) {
		printk(KERN_INFO "vmalloc Setup done\n");
	} else {
		panic("vmalloc Setup failed\n");
		return (1);
	}

	if ((tmp = pmm_init_final(pmm_bootstrap_bitmap_addr_get())) == 0) {
		printk(KERN_INFO "Physical Memory (Final) Manager Setup done\n");
	} else {
		printk(KERN_CRIT "Physical Memory (Final) Manager Setup failed\n");
		if (tmp == 1) {
			panic("System could not find any valid memory to setup final physical memory manager, boot aborted\n");
		}
		if (tmp == 2) {
			panic("System could not get enough physical memory to store the physical memory manager's data structure, boot aborted\n");
		}
		return (1);
	}

	if ((tmp = mem_cache_init()) == 0) {
		printk(KERN_INFO "Heap setup done, kmalloc ready\n");
	} else {
		if (tmp == 1) {
			panic("System could not find any valid memory to setup kernel heap, boot aborted\n");
		} else if (tmp == 2) {
			panic("Couldn't map physical to virtual address, boot aborted\n");
		}
	}

	pmm_test_small();

	return (0);
}