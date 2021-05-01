#include <kfs/mem.h>
#include <kfs/multiboot.h>
#include <kfs/kernel.h>
#include <kfs/pages.h>
/*
    0x20000 = 131072d = 4Gb worth of pages as bit
    4Gb = 1048576 pages / 8 bit per char = 131072
	128 Ko
*/
/*
	pbitmap stand for physical (memory) bitmap
*/
unsigned char	pbitmap[BIT_MAP_SIZE];

/*
	These functions make the arithmetic themselves
	They expect addresses as uint32_t (4 bytes) values aligned on PAGE_SIZE
	Their behavior if you pass them invalid address are undefined
*/
static void		pbitmap_init(void);
static void		pmm_set(uint32_t addr);
static void		pmm_unset(uint32_t addr);

static void		pbitmap_init(void)
{
	for (int i = 0; i < BIT_MAP_SIZE; i++) {
		pbitmap[i] = 0xff;
	}
}

/* mark as taken */
static void		pmm_set(uint32_t addr)
{
	uint32_t			map_index, bit_index;
	unsigned char		tmp;

	map_index = addr / PAGE_SIZE / 8;
	bit_index = addr / PAGE_SIZE % 8;
	tmp = pbitmap[map_index];
	switch (bit_index) {
		case 0:
			pbitmap[map_index] = tmp | 0x80; //1000 0000
			return ;
		case 1:
			pbitmap[map_index] = tmp | 0x40; //0100 0000
			return ;
		case 2:
			pbitmap[map_index] = tmp | 0x20; //0010 0000
			return ;
		case 3:
			pbitmap[map_index] = tmp | 0x10; //0001 0000
			return ;
		case 4:
			pbitmap[map_index] = tmp | 0x08; //0000 1000
			return ;
		case 5:
			pbitmap[map_index] = tmp | 0x04; //0000 0100
			return ;
		case 6:
			pbitmap[map_index] = tmp | 0x02; //0000 0010
			return ;
		case 7:
			pbitmap[map_index] = tmp | 0x01; //0000 0001
			return ;

		default:
			return ;
	}
}

/* mark as free */
static void		pmm_unset(uint32_t addr)
{
	uint32_t			map_index, bit_index;
	unsigned char		tmp;

	map_index = addr / PAGE_SIZE / 8;
	bit_index = addr / PAGE_SIZE % 8;
	tmp = pbitmap[map_index];
	switch (bit_index) {
		case 0:
			pbitmap[map_index] = tmp & 0x7f; //0111 1111
			return ;
		case 1:
			pbitmap[map_index] = tmp & 0xbf; //1011 1111
			return ;
		case 2:
			pbitmap[map_index] = tmp & 0xdf; //1101 1111
			return ;
		case 3:
			pbitmap[map_index] = tmp & 0xef; //1110 1111
			return ;
		case 4:
			pbitmap[map_index] = tmp & 0xf7; //1111 0111
			return ;
		case 5:
			pbitmap[map_index] = tmp & 0xfb; //1111 1011
			return ;
		case 6:
			pbitmap[map_index] = tmp & 0xfd; //1111 1101
			return ;
		case 7:
			pbitmap[map_index] = tmp & 0xfe; //1111 1110
			return ;

		default:
			return ;
	}
}

extern int		pmm_init(void)
{
	t_grub_info			*tmp_grub_info;
	t_mmap				*mmap;
	uint32_t			tmp_addr, tmp_size;

	pbitmap_init();
	tmp_grub_info = grub_info_get();
	if (tmp_grub_info && IS_GFLAG(tmp_grub_info->flags, GFLAG_MMAP)) {
		mmap = (t_mmap *)((uint32_t)tmp_grub_info->mmap_addr + KERNEL_SPACE_V_ADDR);
		
		while ((uint32_t)mmap < (uint32_t)tmp_grub_info->mmap_addr + KERNEL_SPACE_V_ADDR + tmp_grub_info->mmap_length) {

			if (mmap->type == AVAILABLE_MEMORY && (mmap->length_low + mmap->length_high) >= PAGE_SIZE
				&& mmap->base_addr_low < 0xFFFFC000) {
				
				tmp_addr = mmap->base_addr_low;
				tmp_size = mmap->length_low + mmap->length_high;

				if (tmp_addr == 0x100000) {
					//preserve kernel at 1Mb => jump 3Mb further
					//(1Mb bios + 3Mb kernel = 4Mb = 1 page table mapping of our kernel)
					//add 4Mb space for security / scalability
					tmp_size -= ((PAGE_SIZE * 768) + (PAGE_SIZE * PAGE_SIZE));
					tmp_addr += ((PAGE_SIZE * 768) + (PAGE_SIZE * PAGE_SIZE));
				} else if (tmp_addr % PAGE_SIZE) {
					//align memory chunk on PAGE_SIZE
					tmp_size -= tmp_size % PAGE_SIZE;
					tmp_addr -= tmp_addr % PAGE_SIZE;
				}
				if (tmp_size < PAGE_SIZE) {
					//chunk is no longer big enough to store a page after alignement
					mmap = (t_mmap *)((uint32_t)mmap + mmap->size + sizeof(uint32_t));
					continue ;
				}

				// mark memory chunk as free by unseting bit in bitmap
				for (uint32_t i = 0; i < tmp_size; i += PAGE_SIZE) {
					pmm_unset(tmp_addr);
					tmp_addr += PAGE_SIZE;
				}

			}
		 	mmap = (t_mmap *)((uint32_t)mmap + mmap->size + sizeof(uint32_t));
		}

	} else {
		return (1);
	}
	return (0);
}

extern void			*pmm_page_get(mem_type_t mem_type)
{
	unsigned char	tmp;
	uint32_t		addr, i, max;

	if (mem_type == MEM_LOW) {
		max = MEM_LOW_END;
	} else {
		max = MEM_MEDIUM_END;
	}
	max = max / PAGE_SIZE / 8;
	for (i = mem_type == MEM_LOW ? MEM_LOW_START : MEM_MEDIUM_START; i < max; i++) {
		if (pbitmap[i] != 0xff) {
			
			// atleast a page is free here
			tmp = pbitmap[i];
			if ((tmp & 0x80) == 0) {
				addr = (uint32_t)(i * 8 * PAGE_SIZE);
			}
			else if ((tmp & 0x40) == 0) {
				addr = (uint32_t)(i * 8 * PAGE_SIZE + PAGE_SIZE);
			}
			else if ((tmp & 0x20) == 0) {
				addr = (uint32_t)(i * 8 * PAGE_SIZE + (PAGE_SIZE * 2));
			}
			else if ((tmp & 0x10) == 0) {
				addr = (uint32_t)(i * 8 * PAGE_SIZE + (PAGE_SIZE * 3));
			}
			else if ((tmp & 0x08) == 0) {
				addr = (uint32_t)(i * 8 * PAGE_SIZE + (PAGE_SIZE * 4));
			}
			else if ((tmp & 0x04) == 0) {
				addr = (uint32_t)(i * 8 * PAGE_SIZE + (PAGE_SIZE * 5));
			}
			else if ((tmp & 0x02) == 0) {
				addr = (uint32_t)(i * 8 * PAGE_SIZE + (PAGE_SIZE * 6));
			}
			else if ((tmp & 0x01) == 0) {
				addr = (uint32_t)(i * 8 * PAGE_SIZE + (PAGE_SIZE * 7));
			} else {
				printk(KERN_WARNING "Couldn't retrieve physical memory from a free space\n");
				return (NULL);
			}
			pmm_set(addr);
			return ((void *)addr);
		}
	}
	// return null pointer
	printk(KERN_EMERG "System out of physical memory\n");
	return (NULL);
}

extern void			pmm_page_free(void *addr)
{
	if ((uint32_t)addr == NULL || (uint32_t)addr >= 0xFFFFC000) {
		// error addr is invalid
		// 0x101000 = 1Mb + kernel page table
		// 0xFFFFC000 grub reserved space ; about 4Gb - 1 page
		return ;
	}
	if ((uint32_t)addr % PAGE_SIZE != 0) {
		// error addr is invalid
		// addr is not aligned on page bundaries
		return ;
	}
	pmm_unset((uint32_t)addr);
}