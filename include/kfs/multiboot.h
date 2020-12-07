#ifndef __MULTIBOOT_H__
# define __MULTIBOOT_H__

# include <stdint.h>

struct aout_sym_table_info {
	uint32_t	tabsize;
	uint32_t	strsize;
	uint32_t	addr;
	uint32_t	reserved;
};
typedef struct aout_sym_table_info	t_aout_sym_table_info;

struct elf_hdr_table_info {
	uint32_t	num;
	uint32_t	size;
	uint32_t	addr;
	uint32_t	shndx;
};
typedef struct elf_hdr_table_info	t_hdrt_info;

struct grub_modules {
	uint32_t	mod_start;
	uint32_t	mod_end;
	uint32_t	string;
};
typedef struct grub_modules	t_grub_mod;

struct mmap_info {
	uint32_t	size;
	uint32_t	base_addr_low, base_addr_high;
	uint32_t	length_low, length_high;
	uint32_t	type;
};
typedef struct mmap_info	t_mmap;

struct drives {
	uint32_t	size;
	uint8_t		drive_number;
	uint8_t		drive_mode;
	uint16_t	drive_cylinders;
	uint8_t		drive_heads;
	uint8_t		drive_sectors;
	uint16_t	drive_ports;
};
typedef struct drives	t_drives;

struct apm_table {
	uint16_t	version;
	uint16_t	cseg;
	uint16_t	offset;
	uint16_t	cseg_16;
	uint16_t	dseg;
	uint16_t	flags;
	uint16_t	cseg_len;
	uint16_t	cseg_16_len;
	uint16_t	dseg_len;
};
typedef struct apm_table	t_apm_table;

struct palette_color_info {
	uint32_t	framebuffer_palette_addr;
	uint32_t	framebuffer_palette_num_colors;
};
typedef struct palette_color_info	t_palette_color_info;

struct color {
	uint8_t	r;
	uint8_t	g;
	uint8_t	b;
};
typedef struct color	t_color;

struct direct_rgb_color_info {
	uint8_t	framebuffer_red_field_position;
	uint8_t	framebuffer_red_mask_size;
	uint8_t	framebuffer_green_field_position;
	uint8_t	framebuffer_green_mask_size;
	uint8_t	framebuffer_blue_field_position;
	uint8_t	framebuffer_blue_mask_size;
};
typedef struct direct_rgb_color_info t_direct_rgb_color_info;

struct grub_info {
	uint32_t	flags;
	uint32_t	mem_lower;
	uint32_t	mem_upper;
	uint8_t		boot_device[4];
	char		*cmdline;
	uint32_t	mod_count;
	t_grub_mod	*mod_table;
	union {
		t_aout_sym_table_info	aout_sym_table_info;
		t_hdrt_info				elf_hdr_table_info;
	} fmt_info;
	uint32_t	mmap_length;
	uint32_t	mmap_addr;
	uint32_t	drives_length;
	uint32_t	drives_addr;
	uint32_t	config_table;
	char		*bootloader_name;
	t_apm_table	*apm_table;
	uint32_t	vbe_control_info;
	uint32_t	vbe_mode_info;
	uint16_t	vbe_mode;
	uint16_t	vbe_interface_seg;
	uint16_t	vbe_interface_off;
	uint16_t	vbe_interface_len;
	uint32_t	framebuffer_addr_low,framebuffer_addr_high;
	uint32_t	framebuffer_pitch;
	uint32_t	framebuffer_width;
	uint32_t	framebuffer_height;
	uint8_t		framebuffer_bpp;
	uint8_t		framebuffer_type;
	uint32_t	color_info;
};
typedef struct grub_info	t_grub_info;

# define AVAILABLE_MEMORY	0x01
# define UNAVAILABLE_MEMORY	0x02
# define ACPI_MEMORY		0x03
# define RESERVED_MEMORY	0x04
# define DEFECTIVE_MEMORY	0x05

# define GFLAG_MEMINFO		1 << 0
# define GFLAG_DEVICES		1 << 1
# define GFLAG_CMDLINE		1 << 2
# define GFLAG_MODULES		1 << 3
# define GFLAG_FMTAOUT		1 << 4
# define GFLAG_FMTELF		1 << 5
# define GFLAG_MMAP			1 << 6
# define GFLAG_DRIVES		1 << 7
# define GFLAG_BIOS			1 << 8
# define GFLAG_NAME			1 << 9
# define GFLAG_APM			1 << 10
# define GFLAG_VBE			1 << 11
# define GFLAG_FRAMEBUFFER	1 << 12

# define IS_GFLAG(x, y) x & (y)

extern void			grub_meminfo_print(void);
extern void			grub_info_init(uint32_t	*addr);
extern t_grub_info	*grub_info_get(void);

extern t_hdrt_info	*hdrt_info_get(void);

#endif