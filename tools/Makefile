#CC = clang -Weverything

all: mkinitrd

mkinitrd:
	$(CC) -g make_initrd.c -o make_initrd
	$(CC) -g parse_initrd.c -o parse_initrd
	gzip initrd.img

# https://en.wikipedia.org/wiki/Filesystem_Hierarchy_Standard
tree:
	rm -rf fakeroot
	mkdir fakeroot
	cd fakeroot && mkdir -p bin boot dev etc home lib media mnt opt proc root run sbin srv sys tmp usr var

clean:
	rm -f make_initrd ; rm -f initrd.img
