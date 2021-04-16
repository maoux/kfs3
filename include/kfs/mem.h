#ifndef __MEM_H__
# define __MEM_H__

# include <stdint.h>

# define PAGE_DIRECTORY_SIZE	1024
# define PAGE_TABLE_SIZE		1024
# define PAGE_SIZE				0x1000
# define KERNEL_SPACE_V_ADDR	0xC0000000

/*
    0x20000 = 131072d = 4Gb worth of pages as bit
    4Gb = 1048576 pages / 8 bit per char = 131072
	128 Ko
*/
# define BIT_MAP_SIZE			0x20000

typedef struct pmm_stack_unit	t_pmm_stack_unit;
struct pmm_stack_unit {
	uint32_t			size;
	void				*addr;
};


/*first tests, not used in kfs now*/
extern void		paging_enable(uint32_t page_directory);
extern int		paging_init(void);

/*			pmm api	v2 - wip			*/

/*
	should be used at kernel start up
	otherwise, undefined behavior is to expect

	return 0 upon success
	return 1 if multiboot info wasn't available or misformated
*/
extern int		page_manager_init(void);

/*
	O(n)
	retrieve a free page aligned on PAGE_SIZE
	return null pointer if did't find any page available
*/
extern void			*page_manager_get(void);

/*
	O(1)
	addr must be aligned with PAGE_SIZE
*/
extern void			page_manager_free(void *addr);

#endif