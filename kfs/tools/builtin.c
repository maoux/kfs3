#include <kfs/shell.h>
#include <kfs/system.h>
#include <kfs/kernel.h>
#include <kfs/multiboot.h>
#include <kfs/elf.h>
#include <kfs/io.h>
#include <stdlib.h>
#include <kfs/mem.h>

static char		*get_symbol_elf32(uint32_t addr);

extern int		builtin_shutdown(char **opts)
{
	(void)(opts);
	shutdown();
	return (SH_SUCCESS);
}

extern int		builtin_reboot(char **opts)
{
	(void)(opts);
	reboot();
	return (SH_SUCCESS);
}

extern int		builtin_print_options(char **opts)
{
	for (int i = 0; opts[i]; i++) {
		printk("%s\n", opts[i]);
	}
	return (0);
}

static char		*get_symbol_elf32(uint32_t addr)
{
	t_hdrt_info		*mbi_hdr;
	t_Elf32_Shdr	*shdr_cur = NULL;
	char			*shdr_str_table = NULL;
	t_Elf32_Sym		*sym_tab;
	t_Elf32_Sym		*sym = NULL;
	int				size;

	mbi_hdr = hdrt_info_get();
	if (mbi_hdr) {
		shdr_cur = (t_Elf32_Shdr *)((uint32_t)mbi_hdr->addr + KERNEL_SPACE_V_ADDR);
		if (mbi_hdr->num == 0 || mbi_hdr->shndx == 0) {
			return (NULL);
		}
		for (uint32_t i = 0; i < mbi_hdr->num; i++) {
			shdr_cur = (t_Elf32_Shdr *)((uint32_t)mbi_hdr->addr + KERNEL_SPACE_V_ADDR + (mbi_hdr->size * i));
			if (shdr_cur->sh_type == SHT_SYMTAB) {
				sym_tab = (t_Elf32_Sym *)((uint32_t)shdr_cur->sh_addr + KERNEL_SPACE_V_ADDR);
				size = (shdr_cur->sh_size / sizeof(t_Elf32_Sym));
			}
			if (i == mbi_hdr->shndx - 1) {
				shdr_str_table = (char *)((uint32_t)shdr_cur->sh_addr + KERNEL_SPACE_V_ADDR);
			}

		}

		if (shdr_str_table == NULL) {
			return (NULL);
		}

		for (int i = 0; i < size; i++) {
			if (addr > sym_tab[i].st_value
				&& (!sym || sym_tab[i].st_value > sym->st_value)) {
					sym = &(sym_tab[i]);
				}
		}
		if (sym == NULL) {
			return (NULL);
		}
		return (&(shdr_str_table[sym->st_name]));
	}
	return (NULL);
}

extern int		builtin_stack_print(char **opts)
{
	uint32_t	*ebp;
	int			max_frame, frame;

	if (opts && opts[0]) {
		max_frame = atoi(opts[0]);
		if (max_frame <= 0 || max_frame > 50) {
			printk("Error: Invalide argument: %s\n", opts[0]);
			return (SH_ERROR);
		}
	} else {
		max_frame = 10;
	}

	__asm__ volatile ("movl %%ebp, %0" : "=r" (ebp) ::);

	printk("Backtrace:\n");
	for (frame = 0; ebp[0] && frame < max_frame; frame++) {
        printk("  #%d  %08x in %s\n", frame, ebp[1], get_symbol_elf32(ebp[1]));
        ebp = (uint32_t *)(ebp[0]);
    }
	printk("  #%d  %08x in %s\n", frame, ebp[1], get_symbol_elf32(ebp[1]));
	return (SH_SUCCESS);
}
