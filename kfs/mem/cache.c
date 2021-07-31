#include <kfs/kernel.h>
#include <kfs/mem.h>
#include <kfs/pages.h>

/*		TODO to improve this part

	- modify caches structures during runtime (ex: medium become small cache after being free for too long),
		= use a counter in cache structure to identify how many times it was parsed without being used

	- make caches bigger than a single page

	- don't allocate whole space at once but make it extensible depending on user average allocations
		= allocate 1/16 of the space, if it grow fast, make it 1/8, if it grow slowly but reach the max, allocate another 1/16, etc

	- identify the appropriate amount of memory to set in the pool on startup
		= add functions to extend the pool

	- improve bitmap data structure

	- improve linked list
		= make it circular ?
		= add a breakpoint ?
		= make a balanced binary tree instead ?

	- different list for different block sizes ?
*/

cache_list_t		*cache_list = NULL;

/*
	map paddr to vaddr, vaddr is the cache data structure mapping the actual base_addr space of PAGE_SIZE
	this is for alloc of less than one page large
*/
static cache_t		*mem_cache_init_block(cache_t *cache, void *paddr, void *vaddr, size_t block_size,
									cache_type_t type, int options, cache_t *prev, cache_t *next)
{
	if (vmm_map_page(paddr, vaddr, 0) != 0) {
		return (NULL);
	}
	cache->state = FREE;
	cache->type = type;
	cache->block_size = block_size;
	cache->bitmap = 0;
	cache->base_vaddr = vaddr;
	cache->options = options;
	cache->paddr = paddr;
	cache->next = next;
	cache->prev = prev;
	return (cache);
}

//init pool of caches
//return 0 upon success, 1 upon physical memory alloc error, 2 upon virtual memory mapping error
extern int			mem_cache_init()
{
	cache_t			*current_page;
	cache_t			*prev = NULL;
	cache_t			*next = NULL;
	uint32_t		i = 0;
	void			*paddr;
	uint32_t		meta_data_addr = KMALLOC_META_DATA_ADDR;

	//paddr for meta data
	if (!(paddr = pmm_page_get(MEM_MEDIUM))) {
		return (1);
	}
	if (vmm_map_page(paddr, (void *)meta_data_addr, 0) != 0) {
		return (2);
	}
	current_page = (cache_t *)meta_data_addr;
	meta_data_addr += PAGE_SIZE;

	cache_list = (cache_list_t *)current_page;

	for (uint32_t vaddr = KMALLOC_ADDR_SPACE_START; vaddr < KMALLOC_ADDR_SPACE_END; vaddr += PAGE_SIZE) {
		//create new space for meta data
		if (i >= (PAGE_SIZE / sizeof(cache_t)) - 1) {
			if (!(paddr = pmm_page_get(MEM_MEDIUM))) {
				return (1);
			}
			if (vmm_map_page(paddr, (void *)meta_data_addr, 0) != 0) {
				return (2);
			}
			//link next
			(&current_page[i - 1])->next = (cache_t *)meta_data_addr;
			//update current page to manipulate
			current_page = (cache_t *)meta_data_addr;
			meta_data_addr += PAGE_SIZE;
			i = 0;
		}

		//paddr for mapping
		if (!(paddr = pmm_page_get(MEM_MEDIUM))) {
			return (1);
		}

		if (i == (PAGE_SIZE / sizeof(cache_t)) - 2) {
			next == NULL;
		} else {
			next = &(current_page[i + 1]);
		}

		if (i % 3 == 0) { //make a small block cache
			if (mem_cache_init_block(&(current_page[i]), paddr, (void *)vaddr, SMALL_BLOCK_SIZE, SMALL, 0, prev, next) == NULL) {
				return (2);
			}
		} else if (i % 3 == 1) { //make a medium block cache
			if (mem_cache_init_block(&(current_page[i]), paddr, (void *)vaddr, MEDIUM_BLOCK_SIZE, SMALL, 0, prev, next) == NULL) {
				return (2);
			}
		} else { //make a large block cache
			if (mem_cache_init_block(&(current_page[i]), paddr, (void *)vaddr, LARGE_BLOCK_SIZE, SMALL, 0, prev, next) == NULL) {
				return (2);
			}
		}
		prev = &(current_page[i]);
		i++;
	}
	return (0);
}

//take bit position in bitmap and convert it to the actual vaddr of the mapped segment using base_vaddr
static void			*mem_cache_addr_from_position(cache_t *cache, uint8_t position)
{
	if (cache) {
		uint32_t		vaddr = (uint32_t)cache->base_vaddr;

		return ((void *)(vaddr + (uint32_t)cache->block_size * (uint32_t)position));
	}
	return (NULL);
}

//update cache state based on bitmap value
static void			mem_cache_state_update(cache_t *cache)
{
	if (cache->bitmap == 0) {
		cache->state = FREE;
	} else if (cache->block_size == SMALL_BLOCK_SIZE && cache->bitmap == 65535) {
		cache->state = FULL;
	} else if (cache->block_size == MEDIUM_BLOCK_SIZE && cache->bitmap == 255) {
		cache->state = FULL;
	} else if (cache->block_size == LARGE_BLOCK_SIZE && cache->bitmap == 15) {
		cache->state = FULL;
	} else {
		cache->state = PARTIAL;
	}
}

//free a bit in a bitmap given its virtual address
extern void			mem_cache_block_free(cache_t *cache, void *vaddr)
{
	if (cache && vaddr) {
		if ((uint32_t)cache->base_vaddr <= (uint32_t)vaddr) {
			uint32_t	offset = (uint32_t)vaddr - (uint32_t)cache->base_vaddr;
			// printk("offset =%d, base vaddr=%#x, vaddr =%#x\n", offset, (uint32_t)cache->base_vaddr, (uint32_t)vaddr);

			offset /= cache->block_size;

			// printk("offset =%d\n", offset);
			if (offset < 15) {
				cache->bitmap = CLR_BIT(cache->bitmap, offset);
				mem_cache_state_update(cache);
			}
		}
	}
}

//find free or partial cache corresponding the best fit size
extern cache_t		*mem_cache_find_available(size_t size)
{
	cache_list_t	*tmp_cache_list = cache_list;
	cache_t			*cache = NULL;

	if (size >= LARGE_BLOCK_SIZE) {
		return (NULL);
	}else if (size < SMALL_BLOCK_SIZE) {
		while (tmp_cache_list) {
			cache = (cache_t *)tmp_cache_list;
			if ((cache->state == FREE || cache->state == PARTIAL) && cache->block_size == SMALL_BLOCK_SIZE) {
				return (cache);
			}
			tmp_cache_list = (cache_list_t *)cache->next;
		}
	} else if (size < MEDIUM_BLOCK_SIZE) {
		while (tmp_cache_list) {
			cache = (cache_t *)tmp_cache_list;
			if ((cache->state == FREE || cache->state == PARTIAL) && cache->block_size == MEDIUM_BLOCK_SIZE) {
				return (cache);
			}
			tmp_cache_list = (cache_list_t *)cache->next;
		}
	} else if (size < LARGE_BLOCK_SIZE) {
		while (tmp_cache_list) {
			cache = (cache_t *)tmp_cache_list;
			if ((cache->state == FREE || cache->state == PARTIAL) && cache->block_size == LARGE_BLOCK_SIZE) {
				return (cache);
			}
			tmp_cache_list = (cache_list_t *)cache->next;
		}
	}
	return (NULL);
}

//find cache corresponding to given virtual address
extern cache_t		*mem_cache_find_addr(void *vaddr)
{
	cache_list_t	*tmp_cache_list = cache_list;
	cache_t			*cache = NULL;

	while (tmp_cache_list) {
		cache = (cache_t *)tmp_cache_list;
		if ((uint32_t)vaddr >= (uint32_t)(cache->base_vaddr) && (uint32_t)vaddr < ((uint32_t)(cache->base_vaddr) + PAGE_SIZE)) {
			return (cache);
		}
		tmp_cache_list = (cache_list_t *)cache->next;
	}
	return (NULL);
}

//get first free block in the cache's bitmap passed in arguments
//if set is TRUE, set bit as used
//return the corresponding virtual address upon success
//return NULL upon failure or if the cache is full
extern void			*mem_cache_block_get(cache_t *cache, bool set)
{
	if (cache) {
		if (cache->state != FULL) {
			uint8_t		bitmap_size;

			bitmap_size = PAGE_SIZE / cache->block_size;
			for (uint8_t i = 0; i < bitmap_size; i++) {
				if (!VRF_BIT(cache->bitmap, i)) {
					if (set) {
						cache->bitmap = SET_BIT(cache->bitmap, i);
						mem_cache_state_update(cache);
					}
					return (mem_cache_addr_from_position(cache, i));
				}
			}
		}
	}
	//cache is FULL
	return (NULL);
}
 