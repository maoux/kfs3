#ifndef __SHELL_H__
# define __SHELL_H__

#include <stdint.h>

# define BUF_SIZE	256
# define SH_ERROR	1
# define SH_SUCCESS	0

struct shell_cmd {
	const char	*cmd;
	int			(*f)(char **opts);
};
typedef struct shell_cmd	t_shell_cmd;

extern void		shell(void);

/* builins */
extern int		builtin_shutdown(char **opts);
extern int		builtin_reboot(char **opts);
extern int		builtin_stack_print(char **opts);
extern int		builtin_print_options(char **opts);

extern void		stack_print(void);
#endif