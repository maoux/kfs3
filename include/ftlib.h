#ifndef __FTLIB_H__
# define __FTLIB_H__

#include <stddef.h>

extern size_t	nbrlen(int n);
extern size_t	unbrlen(unsigned int n);

extern size_t	nbrlen_base(int n, unsigned int base);
extern size_t	unbrlen_base(unsigned int n, unsigned int base);

#endif