#include <kfs/mem.h>
#include <kfs/pages.h>
#include <kfs/kernel.h>

static void			*(*physical_get_func)(mem_type_t mem_type) = &pmm_bootstrap_page_get;
static void			(*physical_free_func)(void *addr) = &pmm_bootstrap_page_free;

// MAIN PMM STACK

// get - first fit O(1) (for get_page, O(1) litteraly, for get_pages, O(1) until system get totally fragmented,
//   theorically happens after 780 000 get and free of single pages...,
//   to improve in future, for now it will only slow down the system for get_pages if we reach this critical point)

// free - O(1)

// store all free physical space as units defined by their starting address and size in pages number
// to use it efficiently, it is devided in segments of PAGE_SIZE, each containing 256 units of memory
static pmm_stack_t	*pmm_stack = NULL;

//current_unit_index indicate where the stack end (last elem inserted)
static uint8_t		current_unit_index;

//store larger allocated contiguous chunk of physical memory in a linked list
//using the stack data structure to push things in O(1)
//finding an element is O(N)
//freeing an element is O(N)
static pmm_stack_t	*pmm_allocated_list = NULL;
static uint8_t		current_allocated_index;

static pmm_stack_segment_t	*pmm_alloc_segment(void)
{
	pmm_stack_segment_t		*segment;

	segment = vzmalloc(PAGE_SIZE);
	return (segment);
}

static pmm_stack_segment_t	*pmm_get_last_segment(void)
{
	pmm_stack_segment_t		*segment;

	segment = (pmm_stack_segment_t *)pmm_stack;
	if (!segment) {
		return (NULL);
	}
	while (segment->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next) {
		segment = (pmm_stack_segment_t *)(segment->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next);
	}
	return (segment);
}

static pmm_stack_unit_t		*pmm_get_next_from_index(pmm_stack_segment_t *segment, uint8_t index)
{
	if (index >= PMM_STACK_SEGMENT_SIZE - 1) {
		if (pmm_stack) {
			//find our segment in stack by looping in segment linked list
			pmm_stack_segment_t *seg = (pmm_stack_segment_t *)pmm_stack;

			while (seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next) {
				// if our segment is the previous one, return the first unit of current segment
				// as it is the next
				if (seg->pmm_sseg[0].prev == (pmm_stack_unit_t *)segment) {
					return ((pmm_stack_unit_t *)&(seg->pmm_sseg[0]));
				}
				seg = (pmm_stack_segment_t *)seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next;
			}
		}
		//our segment wasn't the previous for any other, implicit means it is the last
		//in linked list, return NULL as next pointed
		return (NULL);
	}
	//internal segment resolution
	return ((pmm_stack_unit_t *)&(segment->pmm_sseg[index + 1]));
}

static pmm_stack_unit_t		*pmm_get_prev_from_index(pmm_stack_segment_t *segment, uint8_t index)
{
	if (index == 0) {
		if (pmm_stack && (pmm_stack_t *)segment != pmm_stack) {
			//find our segment in stack by looping in segment linked list
			pmm_stack_segment_t *seg = (pmm_stack_segment_t *)pmm_stack;

			while (seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next) {
				//if our segment is the next for the last unit of the current segment,
				//the current segment seg is the previous elem for our segment
				if (seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next == (pmm_stack_unit_t *)segment) {
					return ((pmm_stack_unit_t *)&(seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1]));
				}
				seg = (pmm_stack_segment_t *)seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next;
			}
	//		return ((pmm_stack_unit_t *)&(seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1]));
		}
		//segment is equal to the stack starting point OR there is no segment set yet, prev segment is set to NULL
		//(list isn't circular)
		return (NULL);
	}
	//internal segment resolution
	return ((pmm_stack_unit_t *)&(segment->pmm_sseg[index - 1]));
}

static void					pmm_stack_unit_set(pmm_stack_unit_t *unit, uint32_t pages_nb, void *addr,
												pmm_stack_unit_t *next, pmm_stack_unit_t *prev)
{
	if (unit) {
		unit->addr = addr;
		unit->size = pages_nb;
		unit->next = next;
		unit->prev = prev;
	}
}


static void					pmm_segment_unit_set(pmm_stack_segment_t *segment, uint8_t index, uint32_t pages_nb,
												void *addr, pmm_stack_unit_t *next, pmm_stack_unit_t *prev)
{
	if (segment) {
		segment->pmm_sseg[index].addr = addr;
		segment->pmm_sseg[index].size = pages_nb;
		segment->pmm_sseg[index].next = next;
		segment->pmm_sseg[index].prev = prev;
	}
}

//wrapper to switch from physical memory manager bootstrap to final
//Could allow more control on returned physical page without duplicating code
extern void			*pmm_page_get(mem_type_t mem_type)
{
	return ((*physical_get_func)(mem_type));
}

extern void			pmm_page_free(void *addr)
{
	return ((*physical_free_func)(addr));
}

//return 0 upon success
//return 1 if no memory to store data structure was found at all
//return 2 if it couldn't find enough memory to store the entire bitmap
extern int			pmm_init_final(unsigned char *pbitmap)
{
	pmm_stack_segment_t		*current_segment;
	uint8_t					current_sindex;
	uint32_t				current_unit_size;
	void					*current_unit_addr = (void *)NULL;
	pmm_stack_unit_t		*next;
	pmm_stack_unit_t		*prev;

	// alloc first segment of the stack
	current_segment = pmm_alloc_segment();
	if (current_segment == NULL) {
		return (1);
	}
	for (int index = 0; index < PMM_STACK_SEGMENT_SIZE; index++) {
		pmm_segment_unit_set(current_segment, index, 0, NULL, NULL, NULL);
	}

	// set linked list head
	pmm_stack = (pmm_stack_t *)current_segment;

	current_sindex = 0;
	current_unit_size = 0;

	// loop on bitmap to set units of free physical memory
	// and push them onto the stack
	for (int i = 0; i < BIT_MAP_SIZE; i++) {
		for (int j = 7; j >= 0; j--) {

			// bit set => allocated / reserved memory
			// if a unit was being computed, set it and reset current values for next unit
			// == push it onto the stack
			if (((pbitmap[i] >> j) & 1)) {
				if (current_unit_size > 0 && (uint32_t)current_unit_addr != NULL) {
					if (current_sindex == PMM_STACK_SEGMENT_SIZE - 1) {
						//segment end, allocate a new segment
						pmm_stack_segment_t		*tmp_segment;
						tmp_segment = pmm_alloc_segment();
						if (tmp_segment == NULL) {
							return (2);
						}
						for (int index = 0; index < PMM_STACK_SEGMENT_SIZE; index++) {
							pmm_segment_unit_set(tmp_segment, index, 0, NULL, NULL, NULL);
						}
						next = (pmm_stack_unit_t *)tmp_segment;
						prev = pmm_get_prev_from_index(current_segment, current_sindex);
						pmm_segment_unit_set(current_segment, current_sindex, current_unit_size, current_unit_addr, next, prev);
						current_sindex = 0;
						current_segment = tmp_segment;
					} else {
						//set unit and continue to fill up segment
						next = pmm_get_next_from_index(current_segment, current_sindex);
						prev = pmm_get_prev_from_index(current_segment, current_sindex);
						pmm_segment_unit_set(current_segment, current_sindex, current_unit_size, current_unit_addr, next, prev);
						current_sindex++;
					}
					//reset current unit
					current_unit_size = 0;
					current_unit_addr = (void *)NULL;
				}
			} else {
				//bit unset => compute unit
				if (current_unit_size == 0) {
					//set physical memory starting address
					current_unit_addr = (void *)((i * 8 * PAGE_SIZE + (PAGE_SIZE * j)));
				}
				//increase unit size
				current_unit_size++;
			}

		}
	}

	//set index
	current_unit_index = (current_sindex - 1) % PAGE_SIZE;
	current_allocated_index = 0;
	//activate new api
	physical_get_func = &pmm_final_page_get;
	physical_free_func = &pmm_final_page_free;
	return (0);
}

extern void				pmm_unit_print(pmm_stack_unit_t *unit, void *args)
{
	(void)args;
	printk("UNIT %0#x - %d pages, prev addr: %0#x, next addr: %0#x\n", unit->addr, unit->size, unit->prev, unit->next);
}

extern void				pmm_unit_foreach(void (*f)(pmm_stack_unit_t *u, void *args), void *params)
{
	pmm_stack_unit_t	*unit = (pmm_stack_unit_t *)pmm_stack;

	if (!unit) return ;
	do {
		f(unit, params);
		unit = unit->next;
	//printk(KERN_ALERT "%0#x\n", (uint32_t)unit->next);
	} while (unit->next);
}

extern void				pmm_unit_foreach_rev(void (*f)(pmm_stack_unit_t *u, void *args), void *params)
{
	pmm_stack_unit_t	*unit = (pmm_stack_unit_t *)&(((pmm_stack_segment_t *)pmm_stack)->pmm_sseg[current_unit_index]);

	if (!unit) return ;
	do {
		f(unit, params);
		unit = unit->prev;
	} while (unit->prev);
}

extern pmm_stack_unit_t	*pmm_unit_addr_find(void *addr)
{
	pmm_stack_unit_t	*unit = (pmm_stack_unit_t *)pmm_stack;

	if (!unit) return (NULL);
	do {
		if ((uint32_t)unit->addr == (uint32_t)addr) {
			return (unit);
		}
		unit = unit->next;
	} while (unit->next);
	return (NULL);
}

extern uint32_t			pmm_size_get(void *paddr)
{
	pmm_stack_unit_t	*unit;

	unit = (pmm_stack_unit_t *)pmm_allocated_list;

	//search in allocated large chunks
	while (unit) {
		if ((uint32_t)unit->addr == (uint32_t)paddr) {
			return (unit->size * PAGE_SIZE);
		}

		unit = unit->next;
	}
	//search in unalocated memory
	unit = pmm_unit_addr_find(paddr);
	if (unit) {
		return (unit->size * PAGE_SIZE);
	}
	//not a referenced address

	//case 1: it is a PAGE_SIZE aligned address, therefore
	 // it is an unalocated address or a previously allocated page
	if (!(uint32_t)paddr % PAGE_SIZE) {
		return (PAGE_SIZE);
	}
	//case 2: it isn't align, it couldn't have been returned by the pmm API
	 // return 0 as we don't know what size the programmer is assuming the chunk to be
	return (0);
}

extern void			*pmm_final_page_get(mem_type_t mem_type)
{
	pmm_stack_unit_t	*prev;
	void				*paddr;

	if (mem_type == MEM_LOW) {
		pmm_stack_unit_t	*unit = (pmm_stack_unit_t *)pmm_stack;
		if (!unit) {
			panic("Physical memory manager corrupted\n");
			return (NULL);
		}

		do {
			//O(N) (theorically, factually O(1) for a long while before system low memory is fragmented)
			// superior to 1 gives us a page of security
			if ((uint32_t)unit->addr < MEM_LOW_END && unit->size > 1) {
				unit->size -= 1;
				paddr = unit->addr;
				unit->addr = (void *)((uint32_t)paddr + PAGE_SIZE);
				return (paddr);
			}
			unit = unit->next;
		} while (unit);

	} else {
		pmm_stack_segment_t	*segment = pmm_get_last_segment();
		if (!segment) {
			panic("Physical memory manager corrupted\n");
			return (NULL);
		}
		pmm_stack_unit_t	*unit = (pmm_stack_unit_t *)&(segment->pmm_sseg[current_unit_index]);

		//security loop to pop empty unit off the stack
		do {
			prev = unit->prev;
			if (unit->size > 0) {
				//O(1)
				unit->size -= 1;
				paddr = unit->addr;
				if (unit->size == 0) {
					//pop empty stack top
					pmm_stack_unit_set(unit, 0, NULL, NULL, NULL);
					if (current_unit_index == 0) {
						if (prev) {
							prev->next = NULL;
							vfree((void *)unit);
							current_unit_index = PMM_STACK_SEGMENT_SIZE - 1;
						}
					} else {
						current_unit_index--;
					}
				}
				else {
					//update stack top unit
					unit->addr = (void *)((uint32_t)paddr + PAGE_SIZE);
				}
				return (paddr);
			}
			//pop empty stack top
			pmm_stack_unit_set(unit, 0, NULL, NULL, NULL);
			if (current_unit_index == 0) {
				if (prev) {
					prev->next = NULL;
					vfree((void *)unit);
					current_unit_index = PMM_STACK_SEGMENT_SIZE - 1;
				}
			} else {
				current_unit_index--;
			}
			unit = prev;
		} while (unit);

		panic("No more physical memory available\n");
		return (NULL);
	}
	return (NULL);
}

extern void			pmm_final_page_free(void *addr)
{
	if ((uint32_t)addr % PAGE_SIZE || addr == NULL) {
		printk(KERN_ERR "Trying to free physical misaligned memory %#0u\n", (uint32_t)addr);
		return ;
	}

	pmm_stack_unit_t		*next, *prev;
	pmm_stack_segment_t		*current_segment = pmm_get_last_segment();

	if (current_unit_index == PMM_STACK_SEGMENT_SIZE - 1) {
		//segment end, allocate a new segment
		pmm_stack_segment_t		*tmp_segment;

		tmp_segment = pmm_alloc_segment();
		if (tmp_segment == NULL) {
			panic("Physical memory manager corrupted\n");
			return ;
		}
		for (int index = 0; index < PMM_STACK_SEGMENT_SIZE; index++) {
			pmm_segment_unit_set(tmp_segment, index, 0, NULL, NULL, NULL);
		}
		//link top of the stack with the new top segment
		current_segment->pmm_sseg[current_unit_index].next = (pmm_stack_unit_t *)tmp_segment;
		current_unit_index = 0;
		//tmp_segment is the new top, set its first unit
		next = pmm_get_next_from_index(tmp_segment, current_unit_index);
		prev = pmm_get_prev_from_index(tmp_segment, current_unit_index);
		pmm_segment_unit_set(tmp_segment, current_unit_index, 1, addr, next, prev);
	} else {
		//set unit and continue to fill up segment
		if (current_unit_index == 0 && current_segment->pmm_sseg[current_unit_index].size == 0) {
			//do nothing, stack is empty
		} else {
			current_unit_index++;
		}
		next = pmm_get_next_from_index(current_segment, current_unit_index);
		prev = pmm_get_prev_from_index(current_segment, current_unit_index);
		pmm_segment_unit_set(current_segment, current_unit_index, 1, addr, next, prev);
	}
}

extern void			*pmm_pages_get(mem_type_t mem_type, size_t nb_pages)
{
	if (nb_pages == 0) {
		return (NULL);
	}
	if (nb_pages == 1) {
		return (pmm_final_page_get(mem_type));
	}
	if (mem_type == MEM_LOW) {
		//todo return memory low chunk
		return (NULL);
	}
	pmm_stack_unit_t	*ucurrent = (pmm_stack_unit_t *)pmm_stack;

	while (ucurrent) {
		//strictly superior, allow us to never delete middle elems of the stack, quick trick to allow simple multiple contiguous pages allocation
		if (ucurrent->size > nb_pages) {
			void	*paddr = ucurrent->addr;

			//update current unit with new starting physical address and new available pages number (size)
			pmm_stack_unit_set(ucurrent, ucurrent->size - nb_pages, (void *)((uint32_t)ucurrent->addr + PAGE_SIZE * nb_pages), ucurrent->next, ucurrent->prev);

			//store the large chunk to be able to free it later
			if (pmm_allocated_list == NULL) {
				//pmm_allocated_list is empty
				pmm_allocated_list = (pmm_stack_t *)pmm_alloc_segment();
				if (pmm_allocated_list == NULL) {
					panic("System ran out of virtual memory in vmalloc pool, leading to pmm curruption\n");
					return (paddr);
				}
				for (int index = 0; index < PMM_STACK_SEGMENT_SIZE; index++) {
					pmm_segment_unit_set((pmm_stack_segment_t *)pmm_allocated_list, index, 0, NULL, NULL, NULL);
				}
				pmm_stack_segment_t		*seg = (pmm_stack_segment_t *)pmm_allocated_list;
				pmm_stack_unit_t		*next, *prev;
				next = pmm_get_next_from_index(seg, 0);
				prev = NULL; // can't use the function here, it is specialized for stack
				pmm_segment_unit_set(seg, 0, nb_pages, paddr, next, prev);
			} else if (current_allocated_index == PMM_STACK_SEGMENT_SIZE - 1) {
				pmm_stack_segment_t		*seg = pmm_alloc_segment();
				if (seg == NULL) {
					panic("System ran out of virtual memory in vmalloc pool, leading to pmm curruption\n");
					return (paddr);
				}
				for (int index = 0; index < PMM_STACK_SEGMENT_SIZE; index++) {
					pmm_segment_unit_set(seg, index, 0, NULL, NULL, NULL);
				}
				pmm_stack_unit_t		*next, *prev;
				pmm_stack_segment_t		*csegment; //current segment
				csegment = (pmm_stack_segment_t *)pmm_allocated_list;
				while (csegment->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next) {
					csegment = (pmm_stack_segment_t *)csegment->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next;
				}
				next = pmm_get_next_from_index(seg, 0);
				//previous unit is the last unit of the previously last segment
				prev = (pmm_stack_unit_t *)&(csegment->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1]);
				//update previously NULL pointed next at the end of the list
				csegment->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next = (pmm_stack_unit_t *)seg;
				//update index for the new segment
				current_allocated_index = 0;
				pmm_segment_unit_set(seg, 0, nb_pages, paddr, next, prev);
			} else {
				//classic case
				pmm_stack_segment_t		*csegment; //current segment
				csegment = (pmm_stack_segment_t *)pmm_allocated_list;
				while (csegment->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next) {
					csegment = (pmm_stack_segment_t *)csegment->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next;
				}
				pmm_stack_unit_t		*next, *prev;
				//update index
				current_allocated_index++;
				if (current_allocated_index == PMM_STACK_SEGMENT_SIZE - 1) {
					next = NULL; //we are setting last unit of the list, next is NULL
				} else {
					next = pmm_get_next_from_index(csegment, current_allocated_index);
				}
				prev = pmm_get_prev_from_index(csegment, current_allocated_index);
				pmm_segment_unit_set(csegment, current_allocated_index, nb_pages, paddr, next, prev);
			}

			return (paddr);
		}

		ucurrent = ucurrent->next;
	}
	printk(KERN_CRIT "No enough physical memory available to allocate %d Bytes\n", nb_pages * PAGE_SIZE);
	return (NULL);
}

extern void			pmm_pages_free(void *addr)
{
	pmm_stack_unit_t	*ucurrent = (pmm_stack_unit_t *)pmm_allocated_list;
	pmm_stack_unit_t		*next, *prev;

	if (addr == NULL || (uint32_t)addr % PAGE_SIZE) {
		return ;
	}

	while (ucurrent) {
		if ((uint32_t)ucurrent->addr == (uint32_t)addr) {
			//found corresponding allocated address
			//start by updating (removing) the unit of the list
			//todo make something cleaner
			if (ucurrent->prev) {
				(ucurrent->prev)->next = ucurrent->next;
			}
			if (ucurrent->next) {
				(ucurrent->next)->prev = ucurrent->prev;
			}

			//push the unit on the pmm_stack
			pmm_stack_segment_t		*current_segment = pmm_get_last_segment();
			if (current_unit_index == PMM_STACK_SEGMENT_SIZE - 1) {
				//segment end, allocate a new segment
				pmm_stack_segment_t		*tmp_segment;

				tmp_segment = pmm_alloc_segment();
				if (tmp_segment == NULL) {
					panic("No more physical memory\n");
					return ;
				}
				//zeros the new segment
				for (int index = 0; index < PMM_STACK_SEGMENT_SIZE; index++) {
					pmm_segment_unit_set(tmp_segment, index, 0, NULL, NULL, NULL);
				}
				//link top of the stack with the new top segment
				current_segment->pmm_sseg[current_unit_index].next = (pmm_stack_unit_t *)tmp_segment;
				current_unit_index = 0;
				//tmp_segment is the new top, set its first unit
				next = pmm_get_next_from_index(tmp_segment, current_unit_index);
				prev = pmm_get_prev_from_index(tmp_segment, current_unit_index);
				pmm_segment_unit_set(tmp_segment, current_unit_index, ucurrent->size, addr, next, prev);
			} else {
				//set unit and continue to fill up segment
				if (current_unit_index == 0 && current_segment->pmm_sseg[current_unit_index].size == 0) {
					//do nothing, stack is empty
				} else {
					current_unit_index++;
				}
				next = pmm_get_next_from_index(current_segment, current_unit_index);
				prev = pmm_get_prev_from_index(current_segment, current_unit_index);
				pmm_segment_unit_set(current_segment, current_unit_index, ucurrent->size, addr, next, prev);
			}
			return ;
		}
		ucurrent = ucurrent->next;
	}
}

extern void		pmm_test_small(void)
{
	uint32_t	*test = (uint32_t *)pmm_pages_get(MEM_MEDIUM, 10);

	printk(KERN_NOTICE "TESTING VMALLOC physical addr %0#x -> size %d\n", (uint32_t)test, pmm_size_get((void *)test));
	pmm_pages_free((void *)test);
}