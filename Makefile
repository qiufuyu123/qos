

NASM := nasm
CC   := gcc
LD   := ld



CFLAGS = -m32 -fno-builtin -fno-stack-protector -Xlinker -zmuldefs \
             -nostartfiles -lm -c -I kernel/include 
LDFLAGS = -T boot/link.ld -m elf_i386
.PHONY: all
all: clean boot image run

BOOT_DIR:=boot
OBJS=$(BOOT_DIR)/loader.o  kernel/main.o kernel/console.o kernel/io.o kernel/gdt.o kernel/isr.o kernel/qstring.o \
kernel/device/timer.o kernel/mem/malloc.o kernel/mem/memory.o kernel/mem/pmm.o kernel/mem/cleaner.o kernel/mem/vmm.o kernel/task/kthread.o  \
kernel/list.o kernel/task/switch.o kernel/task/sync.o kernel/device/keyboard.o kernel/device/ioqueue.o kernel/task/tss.o \
kernel/mem/ubitmap.o kernel/task/process.o kernel/test/edit.o kernel/syscall.o kernel/usyscall.o kernel/stdio.o kernel/fs/hdd.o \
kernel/fs/fat32/fat32.o kernel/fs/fat32/interface.o kernel/exec/exec.o kernel/kio.o kernel/dev/devfs.o kernel/fs/fd.o  \
kernel/device/clock/clock.o kernel/device/clock/cmos.o \
kernel/fs/qufs/fs.o kernel/fs/qufs/inode.o kernel/qmath.o kernel/hashmap.o kernel/fs/qufs/file.o kernel/fs/qufs/dir.o
boot: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o kernel.elf
image: kernel.elf
	cp kernel.elf image/boot
	grub-mkrescue -o kernel.iso image/
	
run:
	@echo "creating VMWARE disk..."
	-rm kernel-vm-disk.vmdk -rf
	qemu-img convert -f raw -O vmdk hd2.img kernel-vm-disk.vmdk
	qemu -cdrom kernel.iso -hda hd.img -boot d -m 512M
	@echo "COMPILE OK!NOW HAVE FUN IN VMWARE!"
clean:
	-rm boot/*.o
	-rm kernel/*.o
	-rm kernel.elf
	-rm kernel.iso
	-rm image/boot/*.elf
	-rm kernel/fs/*.o
	-rm kernel/fs/qufs/*.o
	-rm kernel/task/*.o
	-rm kernel/device/*.o
cdisk:
	-mount hd.img hd
	-rm hd/d1/*.elf
	umount hd
%.o: %.c
	$(CC) $(CFLAGS)  $< -o $@
%.o: %.asm
	$(NASM) -f elf $< -o $@