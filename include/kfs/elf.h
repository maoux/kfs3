#ifndef __ELF_H__
# define __ELF_H__

# include <stdint.h>

typedef uint32_t	Elf32_Addr;
typedef uint32_t	Elf32_Off;

# define SHT_SYMTAB	2

struct Elf32_Shdr {
	uint32_t   sh_name;
	uint32_t   sh_type;
	uint32_t   sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	uint32_t   sh_size;
	uint32_t   sh_link;
	uint32_t   sh_info;
	uint32_t   sh_addralign;
	uint32_t   sh_entsize;
};
typedef struct Elf32_Shdr 	t_Elf32_Shdr;
 
struct Elf32_Sym {
	uint32_t      st_name;
	Elf32_Addr    st_value;
	uint32_t      st_size;
	unsigned char st_info;
	unsigned char st_other;
	uint16_t      st_shndx;
};
typedef struct Elf32_Sym	t_Elf32_Sym;


#endif