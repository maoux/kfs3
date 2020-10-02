if grub-file --is-x86-multiboot $1; then
	echo multiboot confirmed
	return 0
else
	echo the file is not multiboot
	return 1
fi