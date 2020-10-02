#ifndef __IO_H__
# define __IO_H__

#include <stddef.h>

extern void				outb(size_t port, unsigned char byte);
extern unsigned char	inb(size_t port);

extern void				outw(size_t port, unsigned short int word);
extern unsigned short int	inw(size_t port);

#endif