#ifndef __VGA_H__
# define __VGA_H__

# include <stdint.h>

enum vga_colors {
	VGA_COLOR_BLACK			= 0,
	VGA_COLOR_BLUE			= 1,
	VGA_COLOR_GREEN			= 2,
	VGA_COLOR_CYAN			= 3,
	VGA_COLOR_RED			= 4,
	VGA_COLOR_MAGENTA		= 5,
	VGA_COLOR_BROWN			= 6,
	VGA_COLOR_LIGHT_GREY	= 7,
	VGA_COLOR_DARK_GREY		= 8,
	VGA_COLOR_LIGHT_BLUE	= 9,
	VGA_COLOR_LIGHT_GREEN	= 10,
	VGA_COLOR_LIGHT_CYAN	= 11,
	VGA_COLOR_LIGHT_RED		= 12,
	VGA_COLOR_LIGHT_MAGENTA	= 13,
	VGA_COLOR_LIGHT_BROWN	= 14,
	VGA_COLOR_WHITE			= 15
};

/* multi screens and scroll */
# define SCREEN_NUMBER			4
# define MAX_SCROLL				25
# define SCREEN_META_DATA_SIZE	3


extern void		video_init(uint32_t	*framebuffer_addr,  uint32_t width, uint32_t height);
extern void		textcolor_set(const unsigned char bg, const unsigned char fc);

extern void		putchar(const unsigned char c);
extern void		putstring(const char *str);
extern void		putnbr(int n);
extern void		putunbr(unsigned int n);
extern void		putnbr_base(int n, unsigned int base, uint8_t uppercase);
extern void		putunbr_base(unsigned int n, unsigned int base, uint8_t uppercase);

extern void		screen_next(void);
extern void		screen_prev(void);
extern void		screen_clear(void);

extern void		text_mode_intro_print(void);

extern void		cursor_enable(void);
extern void		cursor_disable(void);
extern void		cursor_move(void);
extern void		cursor_move_up(void);
extern void		cursor_move_right(void);
extern void		cursor_move_down(void);
extern void		cursor_move_left(void);

extern void		buffer_scroll_up(void);
extern void		buffer_scroll_down(void);

#endif