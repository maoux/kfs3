#ifndef __MEM_H__
# define __MEM_H__

# include <stdint.h>

# define KERNEL_SPACE_V_ADDR	0xC0000000

/*
    0x20000 = 131072d = 4Gb worth of pages as bit
    4Gb = 1048576 pages / 8 bit per char = 131072
	128 Ko
*/
# define BIT_MAP_SIZE			0x20000

/*
	Physical memory types and macro
*/
typedef enum mem_type_e		mem_type_t;
enum mem_type_e {
	MEM_LOW,
	MEM_MEDIUM,
	MEM_HIGH
};

# define MEM_LOW_START			0x0
# define MEM_LOW_END			0xfffff		// 1Mb - 1
# define MEM_MEDIUM_START		0x100000	// 1Mb 
# define MEM_MEDIUM_END			0xffbfffff	// 4Gb - 4Mb - 1

# define ZONE_DMA_START			0x0
# define ZONE_DMA_END			0x1000000	// 16Mb

# define __pa(x)	((uint32_t)x - KERNEL_SPACE_V_ADDR)
# define __va(x)	((uint32_t)x + KERNEL_SPACE_V_ADDR)


typedef struct pmm_stack_unit	t_pmm_stack_unit;
struct pmm_stack_unit {
	uint32_t			size;
	void				*addr;
};

/*			pmm api	v2 - wip			*/

/*
	should be used at kernel start up
	otherwise, undefined behavior is to expect

	return 0 upon success
	return 1 if multiboot info wasn't available or misformated
*/
extern int		pmm_init(void);

/*
	O(n)
	retrieve a free page aligned on PAGE_SIZE
	return null pointer if did't find any page available
*/
extern void			*pmm_page_get(mem_type_t mem_type);

/*
	O(1)
	addr must be aligned with PAGE_SIZE
*/
extern void			pmm_page_free(void *addr);

#endif