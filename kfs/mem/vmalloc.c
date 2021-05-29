#include <kfs/mem.h>
#include <kfs/pages.h>
#include <kfs/kernel.h>
#include <string.h>

t_vmalloc_block		*vmalloc_map = (t_vmalloc_block *)VMALLOC_STARTUP_ADDR;
size_t				vmalloc_index;

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

	// printk("%#x\n", (uint32_t)vmalloc_map);
	// allocate and map in virtual memory the data structure' space
	for (size_t i = 0; i < VMALLOC_NB_PAGES; i++) {
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

	//set a breakpoint to allocate in O(1) if the 128Mb
	//of memory haven't been totally allocated atleast once yet
	vmalloc_index = 0;
	return (0);
}

extern void		*vmalloc(size_t size)
{
	// 16 pages, size_t max (unsigned short int 2 bytes)
	if (size < 1) {
		return (NULL);
	}
	uint8_t				nb_pages = (size / PAGE_SIZE) + 1;
	t_vmalloc_block		*current_block;
	uint32_t			*phys_page_addr;
	uint32_t			vaddr;

	// printk("%d %d\n", nb_pages, sizeof(t_vmalloc_block));

	// printk("%d\n", nb_pages);

	if (vmalloc_index < VMALLOC_NB_ENTRIES) {
		current_block = &(vmalloc_map[vmalloc_index]);
		if (current_block->nb_pages >= nb_pages && !current_block->effective_size) {
			//current block is large enough AND free
			size_t	tmp = current_block->nb_pages;

			vmalloc_block_set(current_block, nb_pages, current_block->prev_nb_pages, size);
			vmalloc_index += nb_pages;
			if (vmalloc_index < VMALLOC_NB_ENTRIES) {
				//not yet at the end of vmalloc map, set next block as free
				vmalloc_block_set(&(vmalloc_map[vmalloc_index]), tmp - nb_pages, nb_pages, 0);
			}

			vaddr = (uint32_t)addr_from_index(vmalloc_index - nb_pages);
			for (size_t j = 0; j < nb_pages; j++) {
				current_block += j * sizeof(t_vmalloc_block);
				phys_page_addr = (uint32_t *)pmm_page_get(MEM_MEDIUM);
				
				if (!phys_page_addr) {
					//TODO error
					return (NULL);
				}

				if (vmm_map_page(phys_page_addr, (void *)(vaddr + (j * PAGE_SIZE)), 0)) {
					//TODO error
					return (NULL);
				}
				//printk("%#x %u\n", (uint32_t)current_block, (uint32_t)current_block);
				current_block->physical_addr = phys_page_addr;
			}

			return ((void *)vaddr);
		}
	}
	for (size_t i = 0; i < VMALLOC_NB_ENTRIES;) {
		current_block = &(vmalloc_map[i]);
		//printk("%d ---- %d\n", current_block->nb_pages, current_block->effective_size);
		if (current_block->nb_pages >= nb_pages && !current_block->effective_size) {
			//current block is large enough AND free
			size_t	tmp = current_block->nb_pages;

			vmalloc_block_set(current_block, nb_pages, current_block->prev_nb_pages, size);
			i += nb_pages;
			if (i < VMALLOC_NB_ENTRIES && nb_pages < tmp) {
				//not yet at the end of vmalloc map, set next block as free
				vmalloc_block_set(&(vmalloc_map[i]), tmp - nb_pages, nb_pages, 0);
			}

			vaddr = (uint32_t)addr_from_index(i - nb_pages);
			for (size_t j = 0; j < nb_pages; j++) {
				current_block += j * sizeof(t_vmalloc_block);
				phys_page_addr = (uint32_t *)pmm_page_get(MEM_MEDIUM);
				
				if (!phys_page_addr) {
					//TODO error
					return (NULL);
				}

				if (vmm_map_page(phys_page_addr, (void *)(vaddr + (j * PAGE_SIZE)), 0)) {
					//TODO error
					return (NULL);
				}
				current_block->physical_addr = phys_page_addr;
			}

			return ((void *)vaddr);
		}
		i += current_block->nb_pages;
	}
	//TODO Error no more space in vmalloc vaddr space
	printk(KERN_ERR "No more space for vmalloc\n");
	return (NULL);
}

extern void			vfree(void *vaddr)
{
	if ((uint32_t)vaddr == NULL || (uint32_t)vaddr % PAGE_SIZE
		|| (uint32_t)vaddr < VMALLOC_ADDR_SPACE_START || (uint32_t)vaddr > VMALLOC_ADDR_SPACE_END) {
		//TODO Error invalid addr
		return ;
	}
	size_t				index = index_from_addr(vaddr);
	t_vmalloc_block		*block = &(vmalloc_map[index]);

	// printk("%d\n", index);
	if (block->effective_size == 0) {
		//already free
		return ;
	}

	//free each physical page of the block
	t_vmalloc_block		*current_block;
	for (size_t i = 0; i < block->nb_pages; i++) {
		current_block = block + (i * sizeof(t_vmalloc_block));
		pmm_page_free(current_block->physical_addr);
		current_block->physical_addr = NULL;
	}

	//set block free
	block->effective_size = 0;

	if (index + block->nb_pages < VMALLOC_NB_ENTRIES) {
		t_vmalloc_block		*next_block = &(vmalloc_map[index + block->nb_pages]);
		if (next_block->effective_size == 0) {
			//next block is free
			block->nb_pages += next_block->nb_pages;
		}
	}

	if (block->prev_nb_pages > 0 && block->prev_nb_pages <= index) {
		t_vmalloc_block		*prev_block = &(vmalloc_map[index - block->prev_nb_pages]);
		if (prev_block->effective_size == 0) {
			//prev block is free
			prev_block->nb_pages += block->nb_pages;
		}
	}

}

extern uint32_t		vmalloc_get_size(void *vaddr)
{
	if ((uint32_t)vaddr == NULL || (uint32_t)vaddr % PAGE_SIZE
		|| (uint32_t)vaddr < VMALLOC_ADDR_SPACE_START || (uint32_t)vaddr > VMALLOC_ADDR_SPACE_END) {
			return (0);
	}
	return ((&(vmalloc_map[index_from_addr(vaddr)]))->effective_size);
}

extern uint32_t		vmalloc_get_size_physical(void *vaddr)
{
	if ((uint32_t)vaddr == NULL || (uint32_t)vaddr % PAGE_SIZE
		|| (uint32_t)vaddr < VMALLOC_ADDR_SPACE_START || (uint32_t)vaddr > VMALLOC_ADDR_SPACE_END) {
			return (0);
	}
	return ((&(vmalloc_map[index_from_addr(vaddr)]))->nb_pages * PAGE_SIZE);
}

extern void			*vzmalloc(size_t size)
{
	char	*vaddr = (char *)vmalloc(size);

	if (!vaddr) {
		return (NULL);
	}
	for (size_t i = 0; i < size; i++) {
		vaddr[i] = 0;
	}
	return ((void *)vaddr);
}

extern void			test_vmalloc()
{
	void			*buf[10];

	for (size_t i = 0; i < 10; i++) {
		buf[i] = vmalloc(80);
	}
	for (size_t i = 0; i < 10; i++) {
		printk("%#x = %u | %u \n", (int)buf[i], vmalloc_get_size(buf[i]), vmalloc_get_size_physical(buf[i]));
	}
	vfree(buf[5]);
	vfree(buf[7]);
	vfree(buf[6]);
	t_vmalloc_block *b = &(vmalloc_map[index_from_addr(buf[5])]);
	printk("defrag test: %u(effective size 0=free) %u(nb_pages) %u(prev block nb_pages) %u(phys addr 0=free)\n",
			b->effective_size, b->nb_pages, b->prev_nb_pages, b->physical_addr);

	void *tmp;
	for (size_t i = 0; i < 2200; i++) {
		tmp = vmalloc(65535);
		vfree(tmp);
	}
	for (size_t i = 0; i < VMALLOC_NB_ENTRIES; i++) {
		tmp = vmalloc(1);
		vfree(tmp);
	}
	for (size_t i = 0; i < VMALLOC_NB_ENTRIES; i++) {
		tmp = vmalloc(10000);
		vfree(tmp);
	}
	for (size_t i = 0; i < VMALLOC_NB_ENTRIES; i++) {
		tmp = vzmalloc(4096);
		vfree(tmp);
	}
	for (size_t i = 0; i < VMALLOC_NB_ENTRIES; i++) {
		tmp = vmalloc(15033);
		vfree(tmp);
	}
	char *str = vzmalloc(30);
	str[0] = 'H';
	str[1] = 'e';
	str[2] = 'y';
	str[3] = 0;
	printk("%s | str SIZE = %u | str Phys SIZE = %u\n", str, vmalloc_get_size(str), vmalloc_get_size_physical(str));
	vfree(str);
}