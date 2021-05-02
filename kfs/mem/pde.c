#include <kfs/pages.h>
#include <stddef.h>

extern void			pde_attr_set(pd_entry *e, uint32_t attr)
{
	*e |= attr;
}

extern void			pde_attr_del(pd_entry *e, uint32_t attr)
{
	*e &= ~(attr);
}

extern void			pde_frame_set(pd_entry *e, uint32_t addr)
{
	*e = (*e & ~PDE_FRAME) | addr;
}

extern void			pde_frame_del(pd_entry *e, uint32_t addr)
{
	*e &= ~(addr);
}

inline static pd_entry	*vmm_pd_lookup(t_page_directory *pd, uint32_t vaddr)
{
	if (pd) {
		return ((pd_entry *)pd->pd_entries[PDE_INDEX_GET (vaddr)]);
	}
	return (NULL);
}