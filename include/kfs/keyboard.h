#ifndef __KEYBOARD_H__
# define __KEYBOARD_H__

# include <stdint.h>

# define KEY_MAP_SIZE_2		94
# define MAX_SHORTCUT_SIZE	32

struct shortcut {
	uint32_t	key;
	void		(*callback)(void);
};
typedef struct shortcut		t_shortcut;

/*
* ps2_keyboard_init
* should be called once before using keykoard_loop.
* keyboard_loop behavior might be undefined otherwise
* this function actually init the whole controller
* rather than the keyboard only
* Might be move to ps2 controller file directly
*/
extern uint8_t	ps2_keyboard_init(void);
extern void		keyboard_loop(void);

extern int		shortcut_load(uint32_t shortcut, void (*callback)(void));
extern int		shortcut_delete(uint32_t shortcut);
//extern int		read(char *buffer, uint16_t size);
extern int		read(void);

#endif