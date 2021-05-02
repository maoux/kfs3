#include <kfs/pages.h>
#include <stddef.h>

extern void			pte_attr_set(pt_entry *e, uint32_t attr)
{
	*e |= attr;
}

extern void			pte_attr_del(pt_entry *e, uint32_t attr)
{
	*e &= ~(attr);
}

extern void			pte_frame_set(pt_entry *e, uint32_t addr)
{
	*e = (*e & ~PTE_FRAME) | addr;
}

extern void			pte_frame_del(pt_entry *e, uint32_t addr)
{
	*e &= ~(addr);
}

inline static pt_entry	*vmm_pt_lookup(t_page_table *pt, uint32_t vaddr)
{
	if (pt) {
		return ((pt_entry *)pt->pt_entries[PTE_INDEX_GET (vaddr)]);
	}
	return (NULL);
}