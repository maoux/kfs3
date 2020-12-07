#include <kfs/mem.h>
#include <kfs/multiboot.h>
#include <kfs/kernel.h>

t_pmm_stack_unit	*pmm_stack = NULL;

extern int		pmm_init(void) {
	t_grub_info			*tmp_grub_info;
	t_pmm_stack_unit	*available_mem = NULL;
	t_mmap				*mmap;

	tmp_grub_info = grub_info_get();
	if (tmp_grub_info && IS_GFLAG(tmp_grub_info->flags, GFLAG_MMAP)) {
		mmap = (t_mmap *)((uint32_t)tmp_grub_info->mmap_addr + KERNEL_SPACE_V_ADDR);
		while ((uint32_t)mmap < (uint32_t)tmp_grub_info->mmap_addr + KERNEL_SPACE_V_ADDR + tmp_grub_info->mmap_length) {
			if (mmap->type == AVAILABLE_MEMORY && mmap->base_addr_low != 0) {
				// available_mem->addr = mmap->base_addr_low - (mmap->base_addr_low % 4096);
				// available_mem->size = mmap->length_low + mmap->length_high - ((mmap->length_low + mmap->length_high) % 4096);
				// available_mem->next = pmm_stack;
				printk("coucou\n");
				pmm_stack = available_mem;
			}
		 	mmap = (t_mmap *)((uint32_t)mmap + mmap->size + sizeof(uint32_t));
		}
	} else {
		available_mem->addr = (void *)(0x10000 + 0x1000 * 1024);
		available_mem->size = 0x80000000; //assume 2Gb are available , see if there is better to do here
		available_mem->next = NULL;
		pmm_stack = available_mem;
	}
	return (0);
}