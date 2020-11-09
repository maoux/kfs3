#ifndef __GDT_H__
# define __GDT_H__

# include <stdint.h>

# define GDT_BASE_ADDR	0x00000800
# define GDT_SIZE		07

struct gdtr {
    uint16_t size;
    uint32_t base_addr;
} __attribute__ ((packed));
typedef struct gdtr		t_gdtr;

struct gdt_descriptor {
    uint16_t limit0_15;
    uint16_t base0_15;
    uint8_t base16_23;
    uint8_t access;
    uint8_t limit16_19:4;
    uint8_t flags:4;
    uint8_t base24_31;
} __attribute__ ((packed));
typedef struct gdt_descriptor	t_gdt_descriptor;

/* present bit, must be set to 1 */
# define GDTACESS_PR		0x80

/*
	2 bits wide privileges (0-1-2-3)
	0 = kernel
	3 = user
*/
# define GDTACCESS_PRIVL0	0x00
# define GDTACCESS_PRIVL1	0x20
# define GDTACCESS_PRIVL2	0x40
# define GDTACCESS_PRIVL3	0x60

/*
	descriptor bit
	set : data or code
	clear : system
*/
# define GDTACCESS_S		0x10

/* executable bit 1 if executable code, 0 if data */
# define GDTACCESS_EX		0x08

/*
	direction bit/conforming bit
	Direction bit for data. 0 segment grows up. 1 grows down
	(if 1, offset > limit)
	Conforming bit for code. 0 for restrictive prlv execution
	1 for <= prlv execution
*/
# define GDTACCESS_DC		0x04

/*
	readable/writable bit
	Readable bit for code selector. never writable
	Writable bit for data selector. always readable
*/
# define GDTACCESS_RW		0x02

/*
	accessed bit. should be set to 0,
	CPU set it to 1 when using it
*/
# define GDTACCESS_AC		0x01


/*
	granularity flag bit
	0: 1B granularity (byte granularity)
	1: limit in 4KiB blocks (page granularity)
*/
# define GDTFLAG_GR			0x08

/*
	size flag bit
	0: protected 16 bits mode
	1; protected 32 bits mode
*/
# define GDTFLAG_SZ			0x04

/* Globals */
t_gdtr					_GDTR;

extern void		gdt_init(void);
extern void		gdt_flush(void);

#endif