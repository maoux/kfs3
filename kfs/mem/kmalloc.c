#include <kfs/mem.h>
#include <kfs/pages.h>

extern void		kfree(void *vaddr)
{
	if (vaddr) {
		cache_t		*cache = mem_cache_find_addr(vaddr);
		mem_cache_block_free(cache, vaddr);
	}
}

static void		*kmalloc_large(size_t size)
{
	return (NULL);
}

extern size_t	kmalloc_get_size(void *vaddr)
{
	if (vaddr) {
		cache_t		*cache = mem_cache_find_addr(vaddr);
		if (cache) {
			return (cache->block_size);
		}
	}
	return (0);
}

//TODO add mem type handler and options
extern void		*kmalloc(size_t size)
{
	if (size >= LARGE_BLOCK_SIZE && size <= PAGE_SIZE) {
		return (vmalloc(size));
	} else if (size > PAGE_SIZE) {
		return (kmalloc_large(size));
	}
	cache_t		*cache = mem_cache_find_available(size);
	if (cache) {

//		if (cache->bitmap) printk("bitmap %b - block size %d\n", cache->bitmap, cache->block_size);

		void *vaddr = mem_cache_block_get(cache, TRUE);
		return (vaddr);
	}
	printk("oops\n");
	return (NULL);
}