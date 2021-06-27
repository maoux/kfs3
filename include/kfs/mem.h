#ifndef __MEM_H__
# define __MEM_H__

# include <stdint.h>
# include <stddef.h>

# define KERNEL_SPACE_V_ADDR	0xC0000000 //kernel virtual memory space offset
# define KERNEL_SIZE			0x400000 //4Mb ( 1 page table )
# define SECURITY_SPACE_SIZE	0xC00000 // 12Mb left behind kernel for extensibility

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
# define MEM_MEDIUM_START		0x100000	// 1Mb -> standard memory, if MEM_MEDIUM is requested you might get MEM_LOW returned, to be fixed in further update
# define MEM_MEDIUM_END			0xffbfffff	// 4Gb - 4Mb - 1

# define ZONE_DMA_START			0x0
# define ZONE_DMA_END			0x1000000	// 16Mb

# define __pa(x)	((uint32_t)x - KERNEL_SPACE_V_ADDR) //get physical kernel addr from virtual
# define __va(x)	((uint32_t)x + KERNEL_SPACE_V_ADDR) //get virtual kernel addr from physical



/*				pmm bootstrap api	v2				*/

extern unsigned char	*pmm_bootstrap_bitmap_addr_get(void);

/*
	should be used at kernel start up
	otherwise, undefined behavior is to expect

	return 0 upon success
	return 1 if multiboot info wasn't available or misformated
*/
extern int			pmm_init(void);

/*
	O(n)
	retrieve a free page aligned on PAGE_SIZE
	return null pointer if did't find any page available
*/
extern void			*pmm_bootstrap_page_get(mem_type_t mem_type);

/*
	O(1)
	addr must be aligned with PAGE_SIZE
*/
extern void			pmm_bootstrap_page_free(void *addr);


/*				pmm final api	v2				*/

# define PMM_STACK_SEGMENT_SIZE	256

struct pmm_stack_unit {
	uint32_t				size;
	void					*addr;
	struct pmm_stack_unit	*next;
	struct pmm_stack_unit	*prev;
} __attribute__((packed));

struct pmm_stack_segment {
	struct pmm_stack_unit	pmm_sseg[PMM_STACK_SEGMENT_SIZE];
};

typedef struct pmm_stack_unit		pmm_stack_t;
typedef struct pmm_stack_segment	pmm_stack_segment_t;
typedef struct pmm_stack_unit		pmm_stack_unit_t;

extern int				pmm_init_final(unsigned char *pbitmap);
extern void				pmm_unit_print(pmm_stack_unit_t *unit, void *args);
extern void				pmm_unit_foreach(void (*f)(pmm_stack_unit_t *u, void *args), void *params);
extern void				pmm_unit_foreach_rev(void (*f)(pmm_stack_unit_t *u, void *args), void *params);
extern pmm_stack_unit_t	*pmm_unit_addr_find(void *addr);
extern uint32_t			pmm_size_get(void *paddr);

extern void				*pmm_final_page_get(mem_type_t mem_type);
extern void				*pmm_pages_get(mem_type_t mem_type, size_t nb_pages);

extern void				pmm_final_page_free(void *addr);
extern void				pmm_pages_free(void *addr);



//wrapper to switch from bootstrap manager to final
extern void			*pmm_page_get(mem_type_t mem_type);
extern void			pmm_page_free(void *addr);

//test pmm
extern void		pmm_test_small(void);



/*				VMALLOC				*/

/* used for vmalloc map data structure
	4096 / sizeof(t_vmalloc_block) = 4096 / 16 = 256
	32768 / 256 = 128 pages required to store the entire map of vmalloc address space
	32768 * 16 = 512Kb size of actual vmalloc data structure map
*/
# define VMALLOC_STARTUP_ADDR		0xC1000000//KERNEL_SPACE_V_ADDR + KERNEL_SIZE + SECURITY_SPACE_SIZE // 0xc1000000
# define VMALLOC_NB_ENTRIES			32768 // 128Mb / 4096 (PAGE_SIZE)
# define VMALLOC_NB_PAGES			128

/* used for vmalloc actual function
	VMALLOC_STARTUP_ADDR + VMALLOC_NB_ENTRIES * sizeof(t_vmalloc_block) = 0xC1040000
	vmalloc space is 128 Mb
	VMALLOC_ADDR_SPACE_START + 128Mb = 
*/
# define VMALLOC_ADDR_SPACE_START	0xC1080000//3Gb + 16Mb + 512Kb startup heap address in virtual address space
# define VMALLOC_ADDR_SPACE_END		0xC9080000 //3Gb + 16Mb + 512Kb + 128Mb end of heap addresses in virtual address space

typedef struct s_vmalloc_block	t_vmalloc_block;
struct s_vmalloc_block {
	size_t		nb_pages;
	size_t		prev_nb_pages;
	uint32_t	effective_size;
	uint32_t	*physical_addr;
	uint32_t	align;
}__attribute__((packed));

extern int			vmalloc_init(void);
extern void			*vmalloc(size_t size);
extern void			*vzmalloc(size_t size);
extern void			vfree(void *vaddr);
extern uint32_t		vmalloc_get_size(void *vaddr);
extern uint32_t		vmalloc_get_size_physical(void *vaddr);
extern void			test_vmalloc();

#endif