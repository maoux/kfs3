#include <kfs/system.h>
#include <kfs/io.h>

extern void		shutdown(void)
{
	/* qemu shutdown */
	outw(0x604, 0x2000);
}

extern void		reboot(void)
{

}