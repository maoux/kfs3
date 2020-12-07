#ifndef __MEM_H__
# define __MEM_H__

# include <stdint.h>

# define PAGE_DIRECTORY_SIZE	1024
# define PAGE_TABLE_SIZE		1024
# define KERNEL_SPACE_V_ADDR	0xC0000000

typedef struct pmm_stack_unit	t_pmm_stack_unit;
struct pmm_stack_unit {
	uint32_t			size;
	void				*addr;
	t_pmm_stack_unit	*next;
};


/*first tests, not used in kfs now*/
extern void		paging_enable(uint32_t page_directory);
extern int		paging_init(void);

/*pmm api*/
extern int		pmm_init(void);

#endif