#include <io.h>

extern void		outb(size_t port, unsigned char byte)
{
	__asm__ volatile ("outb %0, %w1;"
		 :
		 : "a" (byte), "Nd" (port)
		);
}

extern void		outw(size_t port, unsigned short int word)
{
	__asm__ volatile ("outw %0, %w1;"
		 :
		 : "a" (word), "Nd" (port)
		);
}

extern unsigned char	inb(size_t port)
{
	unsigned char	byte;

	__asm__ volatile ("inb %1, %0"
			: "=a" (byte)
			: "Nd" (port)
	);
	return (byte);
}


extern unsigned short int	inw(size_t port)
{
	unsigned short int		word;

	__asm__ volatile ("inw %1, %0"
			: "=a" (word)
			: "Nd" (port)
	);
	return (word);
}