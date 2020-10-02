DISKNAME="disk.img"

if [ ! -e $DISKNAME ]; then
	dd if=/dev/zero of=$DISKNAME bs=1024 count=6144
	echo "n\np\n1\n\n\na\nw" |  fdisk $DISKNAME
	FIRST="true"
else
	FIRST="false"
fi

sudo losetup -f $DISKNAME
sudo losetup -f $DISKNAME -o 1048576 #offset 1M for filesystem

LOOP0=$(sudo losetup -a \
		| grep "disk.img)$" \
		| awk '{print $1}' \
		| sed 's/.$//')

grep "en_US" /etc/default/locale
if [ $? -eq 0 ]; then
	LOOP1=$(sudo losetup -a \
		| grep "disk.img), offset" \
		| awk '{print 	$1}' \
		| sed 's/.$//')
else
	LOOP1=$(sudo losetup -a \
		| grep "disk.img), index" \
		| awk '{print 	$1}' \
		| sed 's/.$//')
fi

if [ $FIRST = "true" ]; then
	sudo mke2fs $LOOP1
#	sudo mkdosfs -F32 -f 2 $LOOP1
fi

echo $LOOP0
echo $LOOP1

sudo mount $LOOP1 /mnt

if [ $FIRST = "true" ]; then
	sudo grub-install	--root-directory=/mnt \
						--no-floppy \
						--modules="normal part_msdos ext2 multiboot" \
						$LOOP0
fi

sudo mkdir -p /mnt/boot/grub
sudo cp config/grub.cfg /mnt/boot/grub
sudo cp kfs.bin /mnt/boot

sudo sync
sudo umount $LOOP1
sudo losetup -d $LOOP0
sudo losetup -d $LOOP1
