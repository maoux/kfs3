#include <kfs/kernel.h>
#include <kfs/keyboard.h>
#include <kfs/shell.h>
#include <kfs/vga.h>
#include <string.h>

static void		prompt(void);
static void		shell_cursor_move_right(void);
static void		shell_cursor_move_left(void);
static void		buffer_print(char *buf, uint16_t cursor_position,
							uint8_t end_of_line);

static char		*parse_word(char **line);
static int		line_exec(char *line);

static char		buffer[BUF_SIZE];
static uint16_t	pos;
static uint8_t		cmds_nbr = 5;
static t_shell_cmd	cmds[] = {
	{.cmd = "shutdown", .f = builtin_shutdown},
	{.cmd = "halt", .f = builtin_shutdown},
//	{.cmd = "reboot", .f = builtin_reboot},
	{.cmd = "print-stack", .f = builtin_stack_print},
	{.cmd = "bt", .f = builtin_stack_print},
	{.cmd = "opts", .f = builtin_print_options}
};


static void		prompt(void)
{
	printk("$> ");
}

static void		shell_cursor_move_right(void)
{
	if (pos < BUF_SIZE - 2 && buffer[pos + 2] != 0) {
		pos++;
		buffer_print(&buffer[0], pos, 0);
	}
}

static void		shell_cursor_move_left(void)
{
	if (pos > 0) {
		pos--;
		buffer_print(&buffer[0], pos, 0);
	}
}

static void		buffer_print(char *buf, uint16_t cursor_position,
							uint8_t end_of_line)
{
	size_t		i = 0;

	while (i < BUF_SIZE && buf[i] != 0x0) {
		if (i == cursor_position) {
			textcolor_set(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
			putchar(buf[i]);
			textcolor_set(VGA_COLOR_BLACK, VGA_COLOR_WHITE);
		} else {
			putchar(buf[i]);
		}
		i++;
	}
	if (end_of_line) {
		putchar('\n');
		return ;
	}
	while (i) {
		cursor_move_left();
		i--;
	}
}

static char		*parse_word(char **line)
{
	char		*w;

	while (**line && (**line == ' ' || **line == '\t')) {
		(*line)++;
	}
	w = *line;
	while (**line && **line != ' ' && **line != '\t'
			&& **line != '\n') {
		(*line)++;		
	}
	if (**line) {
		**line = '\0';
		do {
			(*line)++;
		} while (**line && (**line == ' ' || **line == '\t'));
	}
	return (w);
}

#define BUF_SIZE_OPTS (BUF_SIZE / 2 - 2)

static int		line_exec(char *line)
{
	char		*cmd;
	char		*opts[BUF_SIZE_OPTS];
	size_t		i = 0;

	cmd = parse_word(&line);
	while (*line) {
		opts[i] = parse_word(&line);
		i++;
	}
	opts[i] = NULL;
	if (!strcmp(cmd, "")) {
		return (SH_SUCCESS);
	}
	for (uint8_t i = 0; i < cmds_nbr; i++) {
		if (!strcmp(cmd, cmds[i].cmd)) {
		 	return (cmds[i].f(opts));
		}
	}
	printk("Error: %s: command not found\n", cmd);
	return (SH_ERROR);
}

extern void		shell(void)
{
	uint8_t		key = 0;



	shortcut_load(0xe04d, shell_cursor_move_right); /* arrow right */
	shortcut_load(0xe04b, shell_cursor_move_left); /* arrow left */

	cursor_disable();

	while (1) {
		if (key > 0) {
			buffer_print(&buffer[0], BUF_SIZE + 1, 1);
			line_exec(&buffer[0]);
		}
		memset(&buffer[0], 0x0, BUF_SIZE);
		buffer[0] = ' ';
		buffer[1] = ' ';
		pos = 0;
		prompt();
		buffer_print(&buffer[0], pos, 0);
		do {
			key = read();
			if (key != 0 && key != 0x0a) {
				if (key == 0x08) {
					if (pos > 0) {
						memmove(&buffer[pos - 1], &buffer[pos],
								BUF_SIZE - pos);
						buffer[BUF_SIZE - 1] = ' ';
						buffer[BUF_SIZE - 2] = ' ';
						pos--;
					}
				} else {
					if (pos < BUF_SIZE - 2) {
						memmove(&buffer[pos + 1], &buffer[pos],
								BUF_SIZE - pos - 2);
						if (key == '\t') {
							key = ' ';
						}
						buffer[pos] = key;
						buffer[BUF_SIZE - 1] = ' ';
						buffer[BUF_SIZE - 2] = ' ';
						pos++;
					}
				}
				buffer_print(&buffer[0], pos, 0);
			}
		} while (key != 0x0a); /* return key */
	}
}