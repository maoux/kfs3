#include <kfs/kernel.h>

extern void		panic(const char *msg_error)
{
	if (msg_error) {
		printk(KERN_CRIT "%s", msg_error);
	}
	//TODO clean up the stack
	//TODO script to try and avoid the system to totally freeze
	//TODO try to reboot ?
	while (1) ;
}