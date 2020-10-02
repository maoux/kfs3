#include <ftlib.h>

extern size_t	nbrlen(int n)
{
	size_t		len;

	len = 1;
	if (n < 0) {
		len++;
		n *= -1;
	}
	while (n > 9) {
		n /= 10;
		len++;
	}
	return (len);
}

extern size_t	unbrlen(unsigned int n)
{
	size_t		len;

	len = 1;
	while (n > 9) {
		n /= 10;
		len++;
	}
	return (len);
}

extern size_t	nbrlen_base(int n, unsigned int base)
{
	size_t		len;

	len = 1;
	if (n < 0) {
		len++;
		n *= -1;
	}
	while (n >= (int)base) {
		n /= (int)base;
		len++;
	}
	return (len);
}

extern size_t	unbrlen_base(unsigned int n, unsigned int base)
{
	size_t		len;

	len = 1;
	while (n >= base) {
		n /= base;
		len++;
	}
	return (len);
}
