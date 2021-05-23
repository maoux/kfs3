#include <kfs/mem.h>
#include <kfs/pages.h>
#include <kfs/kernel.h>
#include <string.h>

t_vmalloc_block		*vmalloc_map = (t_vmalloc_block *)VMALLOC_STARTUP_ADDR;

static void			vmalloc_block_set(t_vmalloc_block *b, size_t nb_pages, size_t prev_nb_pages, uint32_t effective_size)
{
	b->nb_pages = nb_pages;
	b->prev_nb_pages = prev_nb_pages;
	b->effective_size = effective_size;
}

/*
	return NULL on error
	return virtual address corresponding to given index in vmalloc memory map data structure
*/
static void			*addr_from_index(size_t index)
{
	if (index > VMALLOC_NB_ENTRIES) {
		return (NULL);
	}
	return ((void *)(VMALLOC_ADDR_SPACE_START + index * PAGE_SIZE));
}

/*
	return negative on error
		-1 virtual address out of bound
		-2 virtual address is not aligned
	return index in vmalloc memory map data structure corresponding to given virtual address
*/
static int			index_from_addr(void *vaddr)
{
	uint32_t		addr = (uint32_t)vaddr;

	if (addr < VMALLOC_ADDR_SPACE_START || addr > VMALLOC_ADDR_SPACE_END) {
		return (-1);
	}
	if (addr % PAGE_SIZE) {
		return (-2);
	}
	return ((int)((addr - VMALLOC_ADDR_SPACE_START) / PAGE_SIZE));
}

/*
	return value on error
	return 0 on success
	map enough memory in virtual address space to store a complete map of the 128Mb memory
	reserved for vmalloc api
*/
extern int			vmalloc_init(void)
{
	uint32_t		*phys_page_addr;

	// allocate and map in virtual memory the data structure' space
	for (size_t  i = 0; i < VMALLOC_NB_PAGES; i++) {
		phys_page_addr = (uint32_t *)pmm_page_get(MEM_MEDIUM);
		if (!phys_page_addr) {
			//TODO error
			return (1);
		}
		if (vmm_map_page(phys_page_addr, (void *)((uint32_t)vmalloc_map) + (i * PAGE_SIZE), 0)) {
			//TODO error
			return (1);
		}
	}

	//set data structure at 0
	for (size_t i = 0; i < VMALLOC_NB_ENTRIES; i++) {
		vmalloc_block_set(&(vmalloc_map[i]), 0, 0, 0);
	}

	//set the first memory block
	vmalloc_block_set(&vmalloc_map[0], VMALLOC_NB_ENTRIES, 0, 0);
	return (0);
}