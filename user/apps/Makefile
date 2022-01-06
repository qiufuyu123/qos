.PHONY:all
all:
	mount ../../hd.img ../../hd
	-make -C test all
	sync
	umount ../../hd
clean:
	make -C test clean