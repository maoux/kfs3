#include <kfs/kernel.h>
#include <kfs/vga.h>
#include <stddef.h>

extern void		putnbr(int n)
{
	if (n < 0) {
		putchar('-');
		putnbr(n * -1);
		return ;
	}
	if (n < 9) {
		putchar((char)n + 48);
		return ;
	}
	putnbr(n / 10);
	putchar((char)(n % 10) + 48);
}

extern void		putunbr(unsigned int n)
{
	if (n < 9) {
		putchar((char)n + 48);
		return ;
	}
	putnbr(n / 10);
	putchar((char)(n % 10) + 48);
}

extern void		putnbr_base(int n, unsigned int base, uint8_t uppercase)
{
	static char	uset[16] =	{'0', '1', '2', '3', '4', '5',
							 '6', '7', '8', '9', 'A', 'B',
							 'C', 'D', 'E', 'F'};
	static char	lset[16] =	{'0', '1', '2', '3', '4', '5',
							 '6', '7', '8', '9', 'a', 'b',
							 'c', 'd', 'e', 'f'};

	if (base > 16) {
		return ;
	}
	if (n < 0) {
		putchar('-');
		putnbr_base(-n, base, uppercase);
		return ;
	}
	if (n < (int)base) {
		uppercase ? putchar(uset[n]) : putchar(lset[n]);
		return ;
	}
	putnbr_base(n / base, base, uppercase);
	uppercase ? putchar(uset[n % base]) : putchar(lset[n % base]);	
}

extern void		putunbr_base(unsigned int n, unsigned int base, uint8_t uppercase)
{
	static char	uset[16] =	{'0', '1', '2', '3', '4', '5',
							 '6', '7', '8', '9', 'A', 'B',
							 'C', 'D', 'E', 'F'};
	static char	lset[16] =	{'0', '1', '2', '3', '4', '5',
							 '6', '7', '8', '9', 'a', 'b',
							 'c', 'd', 'e', 'f'};

	if (base > 16) {
		return ;
	}
	if (n < base) {
		uppercase ? putchar(uset[n]) : putchar(lset[n]);
		return ;
	}
	putnbr_base(n / base, base, uppercase);
	uppercase ? putchar(uset[n % base]) : putchar(lset[n % base]);	
}

extern void		putstring(const char *str)
{
	for (size_t i = 0; str[i]; i++) {
		putchar((const unsigned char)str[i]);
	}
}