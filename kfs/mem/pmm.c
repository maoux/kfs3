#include <kfs/mem.h>
#include <kfs/multiboot.h>
#include <kfs/kernel.h>

/*
	for internal use only
	all addr passed through those functions should be aligned on 4096 bundary
	as it is distributed by the pmm_page_alloc and pmm alloc functions.
*/

/* free chunks available */
uint32_t			pmm_stack_index = 0;
t_pmm_stack_unit	pmm_stack[PMM_STACK_MAX];
/* allocated chunk that are bigger than a single page */
uint32_t			pmm_stack_alloc_index = 0;
t_pmm_stack_unit	pmm_stack_alloc[PMM_ALLOC_MAX];

extern int		pmm_init(void)
{
	t_grub_info			*tmp_grub_info;
	t_mmap				*mmap;

	tmp_grub_info = grub_info_get();
	if (tmp_grub_info && IS_GFLAG(tmp_grub_info->flags, GFLAG_MMAP)) {
		mmap = (t_mmap *)((uint32_t)tmp_grub_info->mmap_addr + KERNEL_SPACE_V_ADDR);
		while ((uint32_t)mmap < (uint32_t)tmp_grub_info->mmap_addr + KERNEL_SPACE_V_ADDR + tmp_grub_info->mmap_length) {
			if (mmap->type == AVAILABLE_MEMORY && mmap->base_addr_low != NULL) {
				pmm_stack[pmm_stack_index].addr = (void *)((uint32_t)mmap->base_addr_low - ((uint32_t)mmap->base_addr_low % PAGE_SIZE) + PAGE_SIZE);
				pmm_stack[pmm_stack_index].size = mmap->length_low + mmap->length_high - ((mmap->length_low + mmap->length_high) % PAGE_SIZE) - PAGE_SIZE;
				pmm_stack_index++;
			}
		 	mmap = (t_mmap *)((uint32_t)mmap + mmap->size + sizeof(uint32_t));
		}
	} else {
		pmm_stack[0].addr = (void *)(PAGE_SIZE + PAGE_SIZE * 1024); //preserve kernel physical addr
		pmm_stack[0].size = 0x80000000; //assume 2Gb are available , see if there is better to do here
		pmm_stack_index++;
	}
	return (0);
}

extern void			*pmm_page_get(void)
{
	if (pmm_stack_index <= 0) {
		// error to handle here, PANIC ?
		return (NULL);
	}
	if (pmm_stack[pmm_stack_index].size < PAGE_SIZE) {
		pmm_stack_index--;
		return (pmm_page_get());
	}
	if (pmm_stack[pmm_stack_index].size < PAGE_SIZE * 2) {
		pmm_stack[pmm_stack_index].size = 0;
		return (pmm_stack[pmm_stack_index--].addr);
	}
	//if pmm_stack[pmm_stack_index].size >= PAGE_SIZE * 2
	pmm_stack[pmm_stack_index].size -= PAGE_SIZE;
	pmm_stack[pmm_stack_index].addr = (void *)((uint32_t)pmm_stack[pmm_stack_index].addr + PAGE_SIZE);
	return ((void *)((uint32_t)pmm_stack[pmm_stack_index].addr - PAGE_SIZE));
}

extern void			pmm_page_free(void *addr)
{
	if ((uint32_t)addr < 0x101000 || (uint32_t)addr >= 0xFFFFC000) {
		// error addr is invalid
		// 0x101000 = 1Mb + kernel page table
		// 0xFFFFC000 grub reserved space ; about 4Gb - 1 page
		return ;
	}
	if ((uint32_t)addr % 4096 != 0) {
		// error addr is not aligned on page bundaries
		return ;
	}

	// !!! defragmentation starting after half the stack is full !!!
	// high chance to find a matching address
	// create new room for bigger memory chunk request
	if (pmm_stack_index == (PMM_STACK_MAX / 2)) {
		for (uint32_t i = 0; i < pmm_stack_index; i++) {
			if (((uint32_t)(pmm_stack[i].addr)) + pmm_stack[i].size == (uint32_t)addr) {
				pmm_stack[i].size += PAGE_SIZE;
				return ;
			}
		}
	}
	pmm_stack_index++;
	pmm_stack[pmm_stack_index].size = PAGE_SIZE;
	pmm_stack[pmm_stack_index].addr = addr;
}

//if you want continuous memory - heavier call O(n) (DMA?)
extern void			*pmm_alloc(uint32_t size)
{
	uint32_t		size_pagin;

	if (pmm_stack_index <= 0 || (pmm_stack_alloc_index >= PMM_ALLOC_MAX - 1 && size >= PAGE_SIZE)) {
		// error to handle here, PANIC ?
		return (NULL);
	}
	if (size < PAGE_SIZE) {
		return (pmm_page_get());
	}
	size_pagin = size + (PAGE_SIZE - (size % PAGE_SIZE));
	for (int32_t i = (int32_t)pmm_stack_index; i > -1; i--) {
		if (pmm_stack[i].size > size) {
			pmm_stack_alloc[pmm_stack_alloc_index].size = size;
			pmm_stack_alloc[pmm_stack_alloc_index].addr = pmm_stack[i].addr;
			pmm_stack_alloc_index++;

			pmm_stack[i].size -= size_pagin;
			pmm_stack[i].addr = (void *)((uint32_t)pmm_stack[i].addr + size_pagin);

			return ((void *)((uint32_t)pmm_stack[i].addr - size_pagin));
		}
	}
	// no contiguous space is large enough atm
	// error handling to manage
	return (NULL);
}

/* O(n) => optimization to be done */
extern uint32_t		pmm_get_size(void *addr)
{
	if ((uint32_t)addr < 0x101000 || (uint32_t)addr >= 0xFFFFC000) {
		// error addr is invalid
		// 0x101000 = 1Mb + kernel page table
		// 0xFFFFC000 grub reserved space ; about 4Gb - 1 page
		return (0);
	}
	if ((uint32_t)addr % 4096 != 0) {
		// error addr is not aligned on page bundaries
		return (0);
	}
	for (uint32_t i = 0; i < pmm_stack_alloc_index; i++) {
		if ((uint32_t)(pmm_stack_alloc[i].addr) == (uint32_t)addr) {
			return (pmm_stack_alloc[i].size);
		}
	}
	for (uint32_t i = 0; i < pmm_stack_index; i++) {
		if ((uint32_t)(pmm_stack_alloc[i].addr) == (uint32_t)addr) {
			return (pmm_stack_alloc[i].size);
		}
	}
	//assume this is a single allocated page
	return (PAGE_SIZE);
}

extern void		pmm_free(void *addr)
{
	if ((uint32_t)addr < 0x101000 || (uint32_t)addr >= 0xFFFFC000) {
		// error addr is invalid
		// 0x101000 = 1Mb + kernel page table
		// 0xFFFFC000 grub reserved space ; about 4Gb - 1 page
		return ;
	}
	if ((uint32_t)addr % 4096 != 0) {
		// error addr is not aligned on page bundaries
		return ;
	}
	for (uint32_t i = 0; i < pmm_stack_alloc_index; i++) {
		if ((uint32_t)(pmm_stack_alloc[i].addr) == (uint32_t)addr) {
			if (pmm_stack_index == (PMM_STACK_MAX / 2)) {
				//defragmentation
				for (uint32_t j = 0; j < pmm_stack_index; j++) {
					if (((uint32_t)(pmm_stack[j].addr)) + pmm_stack[j].size == (uint32_t)addr) {
						pmm_stack[j].size += pmm_stack_alloc[j].size;
						return ;
					}
				}
			}
			pmm_stack_index++;
			pmm_stack[pmm_stack_index].size = pmm_stack_alloc[i].size + (PAGE_SIZE - (pmm_stack_alloc[i].size % PAGE_SIZE));
			pmm_stack[pmm_stack_index].addr = addr;
			return ;
		}
	}
	// assume addr is only one page wide
	// actually using pmm_page_free would have been faster here
	pmm_stack_index++;
	pmm_stack[pmm_stack_index].size = PAGE_SIZE;
	pmm_stack[pmm_stack_index].addr = addr;
}