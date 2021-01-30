#ifndef __MEM_H__
# define __MEM_H__

# include <stdint.h>

# define PAGE_DIRECTORY_SIZE	1024
# define PAGE_TABLE_SIZE		1024
# define PAGE_SIZE				0x1000
# define KERNEL_SPACE_V_ADDR	0xC0000000

//These are arbitrary values choosen to be high enough to contain
//average demands on a 32bits simple system
//TODO test to find better values
# define PMM_STACK_MAX			0xC000 //12 pages
# define PMM_ALLOC_MAX			0xC000

typedef struct pmm_stack_unit	t_pmm_stack_unit;
struct pmm_stack_unit {
	uint32_t			size;
	void				*addr;
};


/*first tests, not used in kfs now*/
extern void		paging_enable(uint32_t page_directory);
extern int		paging_init(void);

/*			pmm api				*/

extern int		pmm_init(void);

/*
	one by one page allocator 
	alloc : O(1) as long there is memory space
	otherwise panic
	free : O(1) as long there is not PMM_STACK_MAX
	ramdomly free addresses else start doing defragmentation

	- !!!TODO improve free system to avoid leaks and defragment more efficiently!!!
*/
extern void		*pmm_page_get(void);
extern void		pmm_page_free(void *addr);

/*
	one by one page allocator 
	alloc : O(1) as long there is memory space
	otherwise panic
	free : O(n)

	- !!!TODO improve free system to avoid leaks and defragment more efficiently!!!
*/
extern void		*pmm_alloc(uint32_t size);
extern void		pmm_free(void *addr);
extern uint32_t	pmm_get_size(void *addr);

#endif