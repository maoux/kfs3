#include <keyboard.h>
#include <vga.h>
#include <kfs/kernel.h>
#include <kfs/io.h>
#include <kfs/ps2_controller.h>
#include <kfs/system.h>

static uint8_t		shift = 0;
static uint8_t		ctrl = 0;
static uint8_t		alt = 0;

static char			print_key(uint32_t key);
static uint8_t		current_shortcut_nbr;
static t_shortcut	shortcut_map[MAX_SHORTCUT_SIZE];

/*US QWERTY standard keyboard set 2 lower case*/
static const unsigned char	key_map_2_lower[KEY_MAP_SIZE_2] = {
	0x0,
	0x0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
	0x08, '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	0x0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0x0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0x0,
	'*', 0x0, ' ', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, '7', '8', '9',
	'-', '4', '5', '6',
	'+', '1', '2', '3',
			  '0', '.', 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

/*US QWERTY standard keyboard set 2 upper case*/
static const unsigned char	key_map_2_upper[KEY_MAP_SIZE_2] = {
	0x0,
	0x0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0x08,
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
	0x0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
	0x0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',
	0x0, '*', 0x0, ' ', 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
	0x0, '7', '8', '9',
	'-', '4', '5', '6',
	'+', '1', '2', '3',
			  '0', '.', 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

static char		print_key(uint32_t key)
{
	for (uint8_t i = 0; i < current_shortcut_nbr; i++) {
		if (shortcut_map[i].key == key) {
			shortcut_map[i].callback();
			return (0);
		}
	}
	switch (key) {
		case 0x2a:
		case 0x36:
			shift = 1;
			return (0);
		case 0xaa:
		case 0xb6:
			shift = 0;
			return (0);
		case 0x1d:
			ctrl = 1;
			return (0);
		case 0x9d:
			ctrl = 0;
			return (0);
		case 0x38:
			alt = 1;
			return (0);
		case 0xb8:
			alt = 0;
			return (0);
		default:
			if (key < KEY_MAP_SIZE_2) {
				if (shift) {
					return (key_map_2_upper[key]);
				}
				else {
					return (key_map_2_lower[key]);
				}
			}
			return (0);
	}
}

/*
* TODO add usb legacy check and ACPI (FADT) check
*/
extern uint8_t		ps2_keyboard_init(void)
{
	uint8_t		response;
	uint8_t		config;
	uint8_t		port1_status = 0x01;
	uint8_t		port2_status = 0x01;

	/* disable channel 1 ps2 controller */
	send_command(0x64, 0xad, 0x00, 0, 0);
	/* if exist, disable channel 2 */
	send_command(0x64, 0xa7, 0x00, 0, 0);

	/* flush output buffer */
	inb(0x60);

	/* read controller configuration byte */
	wait_ps2_to_write();
	config = send_command(0x64, 0x20, 0x00, 0, 1);
	//config &= 0xbc; //0b1011100 clear translation and interupts


	/* perform self test */
	wait_ps2_to_write();
	response = send_command(0x64, 0xaa, 0, 0, 1);
	// if (response == 0x55) {
	// 	printk(KERN_INFO "Test PS/2 Controller passed\n");
	// }
	if (response == 0xfc) {
		return (1);
	}

	/*
		write configuration byte here because
		self test might overwrite it
	*/
	//wait_ps2_to_write();
	//send_command(0x64, 0x60, config, 1, 0);

	/* determine if there are 2 channels */
	if (config & 0x20) {
		wait_ps2_to_write();
		send_command(0x64, 0xa8, 0x00, 0, 0);
		wait_ps2_to_write();
		config = send_command(0x64, 0x20, 0x00, 0, 1);
		if (config & 0x20) {
			printk(KERN_INFO "PS/2 controller doesn't support dual channels\n");
		}
		else {
			printk(KERN_INFO "PS/2 controller support dual channels\n");
			wait_ps2_to_write();
			send_command(0x64, 0xa7, 0x00, 0, 0);
		}
	}

	/*test ps/2 ports*/
	wait_ps2_to_write();
	port1_status = send_command(0x64, 0xab, 0x00, 0, 1);
	if ((config & 0x20) == 0) {
		wait_ps2_to_write();
		port2_status = send_command(0x64, 0xa9, 0x00, 0, 1);
	}
	if (port1_status != 0x00 && port2_status != 0x00) {
		/* error no functionning ports left */
		return (1);
	}

	/* enable ports */
	if (port1_status == 0x00) {
		wait_ps2_to_write();
		send_command(0x64, 0xae, 0x00, 0, 0);
	}
	if (port2_status == 0x00) {
		wait_ps2_to_write();
		send_command(0x64, 0xa8, 0x00, 0, 0);
	}

	/* enable interupts */
	//config |= 0x03;
	//wait_ps2_to_write();
	//send_command(0x64, 0x60, config, 1);

	/* set scan code set to 2 */
	response = send_command(0x60, 0xf0, 0x02, 1, 1);
	while (response == 0xfe) {
		wait_ps2_to_read();
		response = send_command(0x60, 0xf0, 0x02, 1, 1);
	}
	printk(KERN_INFO "Current scan code set : %d\n", 2);

	current_shortcut_nbr = 0;
	shortcut_load(0x3d, screen_next); //f2
	shortcut_load(0x3c, screen_prev); //f3
	shortcut_load(0x3f, buffer_scroll_up); //f5
	shortcut_load(0x40, buffer_scroll_down); //f6
	/* arrows */
	// load_shortcut(0xe048, cursor_move_up);
	// load_shortcut(0xe04d, cursor_move_right);
	// load_shortcut(0xe050, cursor_move_down);
	// load_shortcut(0xe04b, cursor_move_left);
	return (0);
}

extern int		shortcut_load(uint32_t shortcut, void (*callback)(void))
{
	for (uint8_t i = 0; i < current_shortcut_nbr; i++) {
		if (shortcut_map[i].key == shortcut) {
			shortcut_map[current_shortcut_nbr].callback = callback;
			return (0);
		}
	}
	if (current_shortcut_nbr < MAX_SHORTCUT_SIZE) {
		shortcut_map[current_shortcut_nbr].key = shortcut;
		shortcut_map[current_shortcut_nbr].callback = callback;
		current_shortcut_nbr++;
	} else {
		return (1);
	}
	return (0);
}

/*
	assume key wont be more than 4 bytes wide ,
	doesn't handle this behavior yet
*/
//extern int		read(char *buffer, uint16_t size)
extern int		read(void)
{
	uint32_t		key, tmp = 0;

//	for (uint16_t i = 0; i < size;) {
	key = 0;
	wait_ps2_to_read();
	key = inb(0x60);
	if (key == 0xe0) {
		wait_ps2_to_read();
		while ((tmp = inb(0x60)) == 0xe0) {
			wait_ps2_to_read();
		}
		key = key << 8;
		key |= tmp;
	}
	tmp = print_key(key);
// 		if ((tmp = print_key(key)) > 0) {
// 			buffer[i++] = (char)tmp;
// 		}
//	}
	return (tmp);
}