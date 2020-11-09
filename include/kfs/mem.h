#ifndef __MEM_H__
# define __MEM_H__

# include <stdint.h>

# define PAGE_DIRECTORY_SIZE	1024
# define PAGE_TABLE_SIZE		1024

extern void		paging_enable(uint32_t page_directory);
extern int		paging_init(void);

#endif