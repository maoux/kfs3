#include <kfs/mem.h>

uint32_t		page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(4096)));
uint32_t		first_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(4096)));

extern int		paging_init(void)
{
	for (int i = 0; i < PAGE_DIRECTORY_SIZE; i++) {
		page_directory[i] = 0x00000002;
	}
	for (int i = 0; i < PAGE_TABLE_SIZE; i++) {
		first_page_table[i] = (i * 0x1000) | 3;
	}
	//first_page_table[1023] = 0xb8000 | 3;
	page_directory[0] = (uint32_t)first_page_table | 3;
	page_directory[768] = (uint32_t)first_page_table | 3;
	paging_enable((uint32_t)page_directory);
	page_directory[0] = 0;
	return (0);
}