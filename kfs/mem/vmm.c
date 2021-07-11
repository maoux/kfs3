#include <kfs/mem.h>
#include <kfs/pages.h>
#include <kfs/kernel.h>
#include <string.h>

t_page_directory	*global_pd;
t_page_directory	*_current_pd;
/* recursive page directory to page table itself */
t_page_directory	*kernel_page_dir = (t_page_directory *) 0xFFFFF000;


extern t_page_info			vmm_virt_to_page_index(void *addr)
{
	t_page_info		pginf;
	uint32_t		vaddr = (uint32_t) addr;

	vaddr &= ~0xFFF;
	pginf.pagetable = vaddr >> 22;
	pginf.page = (vaddr << 10) >> 22;
	return (pginf);
}

extern void					vmm_pd_switch(void *pd_vaddr)
{
	if ((uint32_t)pd_vaddr < 0x100000) {
		return ;
	}
	_current_pd = (t_page_directory *)pd_vaddr;
}

extern t_page_directory		*vmm_cr3_get(void)
{
	t_page_directory		*cr3;

	__asm__ volatile ("movl %%cr3, %0" : "=r" (cr3) ::);
	return (cr3);
}

extern void					vmm_cr3_set(t_page_directory *new_pd)
{

	__asm__ volatile ("movl %0, %%cr3" :: "r" (new_pd) :);
}

extern t_page_directory		*vmm_pd_get(void)
{
	return (_current_pd);
}

extern int			vmm_init(void *pd_vaddr)
{
	if ((uint32_t)pd_vaddr < 0x100000) {
		return (1);
	}
	global_pd = pd_vaddr;
	vmm_pd_switch(global_pd);

	pd_entry	*recursive_pd_mapping = &global_pd->pd_entries[PDE_INDEX_GET (0xFFC00000)];

	pde_attr_set(recursive_pd_mapping, PDE_PRESENT);
	pde_attr_set(recursive_pd_mapping, PDE_WRITABLE);
	pde_frame_set(recursive_pd_mapping, ((uint32_t)global_pd - KERNEL_SPACE_V_ADDR));
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

extern void			vmm_free_page(pt_entry *e)
{
	pmm_page_free((void *) PTE_FRAME_GET (e));
	pte_attr_del(e, PTE_PRESENT);
}

/*
	wrapping asm function to be able to work with addr in future
*/
extern void			vmm_flush_tlb_entry(uint32_t *addr)
{
	flush_TLB((uint32_t)addr);
}

extern int			vmm_map_page(void *paddr, void *vaddr, uint32_t attr)
{
	t_page_info 		pginf = vmm_virt_to_page_index(vaddr);
	t_page_directory	*page_directory = vmm_pd_get();
	pd_entry			*pde = &page_directory->pd_entries[PDE_INDEX_GET ((uint32_t)vaddr)];
	t_page_table		*virtual_table = NULL;

	if (!attr) {
		//default attribut for kernel pages
		attr = PTE_PRESENT | PTE_WRITABLE;
	}

	if (!PDE_IS_PRESENT(*pde)) {
		t_page_table *new_table = (t_page_table *) pmm_page_get(MEM_MEDIUM);
		if (!new_table) {
			panic("System could not get anymore physical memory inside virtual memory manager\n");
			return (1);
		}

		virtual_table = (t_page_table *) (0xFFC00000 + (pginf.pagetable * PAGE_SIZE));
		pd_entry		*entry = &kernel_page_dir->pd_entries[pginf.pagetable];

		pde_attr_set(entry, attr);
		pde_frame_set(entry, (uint32_t)new_table);
		memset(virtual_table, 0, sizeof(t_page_table));
		
		pt_entry	*pte = &virtual_table->pt_entries[pginf.page];

		pte_frame_set(pte, (uint32_t)paddr);
		pte_attr_set(pte, attr);
		return (0);
	}
	virtual_table = (t_page_table *) (0xFFC00000 + (pginf.pagetable * PAGE_SIZE));
	pt_entry 		*pte = &virtual_table->pt_entries[PTE_INDEX_GET ((uint32_t)vaddr)];

	pte_frame_set(pte, (uint32_t)paddr);
	pte_attr_set(pte, attr);
	return (0);
}

extern int		vmm_unmap_page(void *vaddr)
{
	if ((uint32_t)vaddr % PAGE_SIZE) {
		printk(KERN_ERR "Trying to unmap a missaligned address %#0x\n", vaddr);
		return (1);
	}
	
	t_page_directory	*page_directory = vmm_pd_get();
	t_page_info 		pginf = vmm_virt_to_page_index(vaddr);
	pd_entry			*pde = &page_directory->pd_entries[PDE_INDEX_GET ((uint32_t)vaddr)];

	if (!PDE_IS_PRESENT (*pde)) {
		printk(KERN_ERR "Trying to unmap a page in an already zeroed page directory entry\n");
		return (2);
	}

	t_page_table		*virtual_table = (t_page_table *) (0xFFC00000 + (pginf.pagetable * PAGE_SIZE));
	pt_entry			*pte = &virtual_table->pt_entries[PTE_INDEX_GET ((uint32_t)vaddr)];

	if (!PTE_IS_PRESENT (*pte)) {
		printk(KERN_ERR "Trying to unmap an already zeroed page table entry\n");
		return (3);
	}

	vmm_free_page(pte);
	return (0);
}