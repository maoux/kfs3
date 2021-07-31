#ifndef __KERNEL_H__
# define __KERNEL_H__

# include <stddef.h>

# define NULL 0

typedef enum bool_e bool;
enum bool_e {
	FALSE = 0,
	TRUE = 1
};

# define KERN_NONE		"8"
# define KERN_EMERG		"7"
# define KERN_ALERT		"6"
# define KERN_CRIT		"5"
# define KERN_ERR		"4"
# define KERN_WARNING	"3"
# define KERN_NOTICE	"2"
# define KERN_INFO		"1"
# define KERN_DEBUG		"0"

# define DEFAULT_PRIORITY_LEVEL	"1"

# define TYPE_CHAR		1
# define TYPE_SHORT		2
# define TYPE_LONG		3
# define TYPE_LONG_LONG	4
# define TYPE_DEFAULT	0

int					printk(const char *fmt, ...);
extern int			mm_init(void *page_directory_vaddr);
extern void			*vmalloc(size_t size);
extern void			vfree(void *vaddr);

extern void			panic(const char *msg_error);

#endif