#include <kfs/mem.h>
#include <kfs/pages.h>
#include <kfs/kernel.h>

static void			*(*physical_get_func)(mem_type_t mem_type) = &pmm_bootstrap_page_get;

// MAIN PMM STACK (act more like a queue (LIFO) at the moment... get from elem 0, free at elem length + 1)

// get - first fit O(1) (for get_page, O(1) litteraly, for get_pages, O(1) until system get totally fragmented,
//   theorically happens after 780 000 get and free of single pages...,
//   to improve in future, for now it will only slow down the system for get_pages if we reach this critical point)

// free - O(K) with K = segment number, PAGE_SIZE/sizeof(t_pmm_stack_unit) times < from elem number N as K = N / (PAGE_SIZE/sizeof(t_pmm_stack_unit))

// store all free physical space as units defined by their starting address and size in pages number
// to use it efficiently, it is devided in segments of PAGE_SIZE, each containing 256 units of memory
static t_pmm_stack	*pmm_stack = NULL;
//current_unit_index indicate where the stack end (last elem inserted + 1)
//increase efficiency to free
static uint8_t		current_unit_index = 0;

static t_pmm_stack_segment	*pmm_alloc_segment(void)
{
	t_pmm_stack_segment		*segment;

	segment = vzmalloc(PAGE_SIZE);
	return (segment);
}

static t_pmm_stack_unit		*pmm_get_next_from_index(t_pmm_stack_segment *segment, uint8_t index)
{
	if (index >= PMM_STACK_SEGMENT_SIZE - 1) {
		if (pmm_stack) {
			//find our segment in stack by looping in segment linked list
			t_pmm_stack_segment *seg = (t_pmm_stack_segment *)pmm_stack;

			while (seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next) {
				// if our segment is the previous one, return the first unit of current segment
				// as it is the next
				if (seg->pmm_sseg[0].prev == (t_pmm_stack_unit *)segment) {
					return ((t_pmm_stack_unit *)&(seg->pmm_sseg[0]));
				}
				seg = (t_pmm_stack_segment *)seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next;
			}
		}
		//our segment wasn't the previous for any other, implicit means it is the last
		//in linked list, return NULL as next pointed
		return (NULL);
	}
	//internal segment resolution
	return ((t_pmm_stack_unit *)&(segment->pmm_sseg[index + 1]));
}

static t_pmm_stack_unit		*pmm_get_prev_from_index(t_pmm_stack_segment *segment, uint8_t index)
{
	if (index == 0) {
		if (pmm_stack && (t_pmm_stack *)segment != pmm_stack) {
			//find our segment in stack by looping in segment linked list
			t_pmm_stack_segment *seg = (t_pmm_stack_segment *)pmm_stack;

			while (seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next) {
				//if our segment is the next for the last unit of the current segment,
				//the current segment seg is the previous elem for our segment
				if (seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next == (t_pmm_stack_unit *)segment) {
					return ((t_pmm_stack_unit *)&(seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1]));
				}
				seg = (t_pmm_stack_segment *)seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1].next;
			}
	//		return ((t_pmm_stack_unit *)&(seg->pmm_sseg[PMM_STACK_SEGMENT_SIZE - 1]));
		}
		//segment is equal to the stack starting point OR there is no segment set yet, prev segment is set to NULL
		//(list isn't circular)
		return (NULL);
	}
	//internal segment resolution
	return ((t_pmm_stack_unit *)&(segment->pmm_sseg[index - 1]));
}


static void					pmm_stack_unit_set(t_pmm_stack_segment *segment, uint8_t index, uint32_t pages_nb,
												void *addr, t_pmm_stack_unit *next, t_pmm_stack_unit *prev)
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

extern int			pmm_final_init(unsigned char *pbitmap)
{
	t_pmm_stack_segment		*current_segment;
	uint8_t					current_sindex;
	uint32_t				current_unit_size;
	void					*current_unit_addr = (void *)NULL;
	t_pmm_stack_unit		*next;
	t_pmm_stack_unit		*prev;

	// alloc first segment of the stack
	current_segment = pmm_alloc_segment();
	if (current_segment == NULL) {
		//todo error
		return (1);
	}
	for (int index = 0; index < PMM_STACK_SEGMENT_SIZE; index++) {
		pmm_stack_unit_set(current_segment, index, 0, NULL, NULL, NULL);
	}

	// set linked list head
	pmm_stack = (t_pmm_stack *)current_segment;

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
						t_pmm_stack_segment		*tmp_segment;
						tmp_segment = pmm_alloc_segment();
						if (tmp_segment == NULL) {
							//todo error
							return (1);
						}
						for (int index = 0; index < PMM_STACK_SEGMENT_SIZE; index++) {
							pmm_stack_unit_set(tmp_segment, index, 0, NULL, NULL, NULL);
						}
						next = (t_pmm_stack_unit *)tmp_segment;
						prev = pmm_get_prev_from_index(current_segment, current_sindex);
						pmm_stack_unit_set(current_segment, current_sindex, current_unit_size, current_unit_addr, next, prev);
						current_sindex = 0;
						current_segment = tmp_segment;
					} else {
						//set unit and continue to fill up segment
						next = pmm_get_next_from_index(current_segment, current_sindex);
						prev = pmm_get_prev_from_index(current_segment, current_sindex);
						pmm_stack_unit_set(current_segment, current_sindex, current_unit_size, current_unit_addr, next, prev);
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
	current_unit_index = current_sindex;
	return (0);
}

extern void				pmm_unit_print(t_pmm_stack_unit *unit, void *args)
{
	(void)args;
	printk("UNIT %0#x - %d pages, prev addr: %0#x, next addr: %0#x\n", unit->addr, unit->size, unit->prev, unit->next);
}

extern void				pmm_unit_foreach(void (*f)(t_pmm_stack_unit *u, void *args), void *params)
{
	t_pmm_stack_unit	*unit = (t_pmm_stack_unit *)pmm_stack;

	if (!unit) return ;
	do {
		f(unit, params);
		unit = unit->next;
	} while (unit->next);
}