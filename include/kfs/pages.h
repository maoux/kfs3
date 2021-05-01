#ifndef __PAGES_H__
# define __PAGES_H__

# include <stdint.h>
# include <mem.h>

extern void *page_directory;

# define PAGE_DIRECTORY_SIZE	1024
# define PAGE_TABLE_SIZE		1024
# define PAGE_SIZE				4096

typedef enum e_pde_attr			pde_attr;
enum	e_pde_attr {
	PDE_FRAME = 0x7ffff000,					// 1111 1111 1111 1111 1111 0000 0000 0000
	PDE_IGNORED = 256,						// 0001 0000 0000
	PDE_PAGE_SIZE = 128,					// 0000 1000 0000
	// unused bit must be at 0
	PDE_ACCESSED = 32, 						// 0000 0010 0000
	PDE_NOT_CACHABLE = 16,					// 0000 0001 0000
	PDE_WTHROUGH = 8,						// 0000 0000 1000
	PDE_USER = 4,							// 0000 0000 0100
	PDE_WRITABLE = 2,						// 0000 0000 0010
	PDE_PRESENT = 1							// 0000 0000 0001
};


typedef enum e_pte_attr			pte_attr;
enum	e_pte_attr {
	PTE_FRAME = 0x7ffff000,					// 1111 1111 1111 1111 1111 0000 0000 0000
	PTE_GLOBAL = 256,						// 0001 0000 0000
	PTE_PAT = 128,							// 0000 1000 0000
	PTE_DIRTY = 64,							// 0000 0100 0000
	PTE_ACCESSED = 32, 						// 0000 0010 0000
	PTE_NOT_CACHABLE = 16,					// 0000 0001 0000
	PTE_WTHROUGH = 8,						// 0000 0000 1000
	PTE_USER = 4,							// 0000 0000 0100
	PTE_WRITABLE = 2,						// 0000 0000 0010
	PTE_PRESENT = 1							// 0000 0000 0001
};

typedef uint32_t pt_entry;
typedef uint32_t pd_entry;

# define PDE_INDEX_GET(addr)	((addr >> 22) & 0x3ff)
# define PTE_INDEX_GET(addr)	((addr >> 12) & 0x3ff)

# define PDE_IS_PRESENT(e)		(e & PDE_PRESENT)
# define PDE_IS_WRITABLE(e)		(e & PDE_WRITABLE)
# define PDE_IS_USER(e)			(e & PDE_USER)

# define PTE_IS_PRESENT(e)		(e & PTE_PRESENT)
# define PTE_IS_WRITABLE(e)		(e & PTE_WRITABLE)
# define PTE_IS_USER(e)			(e & PTE_USER)

# define PDE_FRAME_GET(e)		(*e & PDE_FRAME)
# define PDE_ATTR_GET(e)		(*e & ~(PDE_FRAME))

# define PTE_FRAME_GET(e)		(*e & PTE_FRAME)
# define PTE_ATTR_GET(e)		(*e & ~(PTE_FRAME))

typedef struct s_page_directory	t_page_directory;
struct s_page_directory {
	uint32_t	pd_entries[PAGE_DIRECTORY_SIZE];
};

typedef struct s_page_table		t_page_table;
struct s_page_table {
	uint32_t	pt_entries[PAGE_TABLE_SIZE];
};

extern int		vmm_init(void *pd);
extern t_page_directory		*vmm_pd_get(void);
extern int			vmm_map_page(void *paddr, void *vaddr, uint32_t attr);

#endif