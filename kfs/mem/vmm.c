#include <kfs/mem.h>
#include <kfs/pages.h>
#include <kfs/kernel.h>
#include <string.h>

t_page_directory	*global_pd;

static void			pde_attr_set(pd_entry *e, uint32_t attr)
{
	*e |= attr;
}

static void			pte_attr_set(pt_entry *e, uint32_t attr)
{
	*e |= attr;
}

static void			pde_attr_del(pd_entry *e, uint32_t attr)
{
	*e &= ~(attr);
}

static void			pte_attr_del(pt_entry *e, uint32_t attr)
{
	*e &= ~(attr);
}

static void			pde_frame_set(pd_entry *e, uint32_t addr)
{
	*e |= addr;
}

static void			pte_frame_set(pt_entry *e, uint32_t addr)
{
	*e |= addr;
}

static void			pde_frame_del(pd_entry *e, uint32_t addr)
{
	*e &= ~(addr);
}

static void			pte_frame_del(pt_entry *e, uint32_t addr)
{
	*e &= ~(addr);
}

extern int			vmm_init(void *pd_vaddr)
{
	if ((uint32_t)pd_vaddr < 0x100000) {
		return (1);
	}
	global_pd = pd_vaddr;
	return (0);
}

extern int			vmm_alloc_page(pt_entry *e)
{
	uint32_t	paddr_page = (uint32_t)pmm_page_get(MEM_MEDIUM);
	if (paddr_page == NULL) {
		return (1);
	}
	pte_attr_set(e, PTE_PRESENT);
	pte_frame_set(e, paddr_page);
	return (0);
}

extern void		vmm_free_page(pt_entry *e)
{
	pmm_page_free((void *) PTE_FRAME_GET (e));
	pte_attr_del(e, PTE_PRESENT);
}

inline static pt_entry	*vmm_pt_lookup(t_page_table *pt, uint32_t vaddr)
{
	if (pt) {
		return ((pt_entry *)pt->pt_entries[PTE_INDEX_GET (vaddr)]);
	}
	return (NULL);
}

inline static pd_entry	*vmm_pd_lookup(t_page_directory *pd, uint32_t vaddr)
{
	if (pd) {
		return ((pd_entry *)pd->pd_entries[PDE_INDEX_GET (vaddr)]);
	}
	return (NULL);
}

extern t_page_directory		*vmm_pd_get(void)
{
	return (global_pd);
}

extern int			vmm_map_page(void *paddr, void *vaddr, uint32_t attr)
{
	t_page_directory	*current_pd = vmm_pd_get();										//page directory
	pd_entry			*pde = &current_pd->pd_entries[PDE_INDEX_GET ((uint32_t)vaddr)];  //page directory entry = page table

	if (!PDE_IS_PRESENT ((uint32_t)pde)) {
		// page table is not yet set
		t_page_table	*pt = (t_page_table *)pmm_page_get(MEM_MEDIUM); //new page table
		if (pt == NULL) {
			return (1); //!!! no more space !!!
		}
		memset((void *)pt, 0, sizeof(pt));
		pde_attr_set(pde, PDE_PRESENT | PDE_WRITABLE);
		pde_frame_set(pde, (uint32_t)pt);
	}
	t_page_table	*current_pt = (t_page_table *) PDE_FRAME_GET (pde);
	pt_entry		*pte = &current_pt->pt_entries[PTE_INDEX_GET ((uint32_t)vaddr)]; //page table entry
	if (PTE_IS_PRESENT ((uint32_t)pte)) {
		// page table entry is already present
		return (2);// !!!
	}
	if (attr == 0) {
		//default kernel attr
		attr = PTE_PRESENT | PTE_WRITABLE;
	}
	pte_attr_set(pte, attr);
	pte_frame_set(pte, (uint32_t)paddr);
	return (0);
}