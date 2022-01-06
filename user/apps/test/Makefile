NASM := nasm
CC   := gcc
LD   := ld



CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
             -nostartfiles -nodefaultlibs  -c -I ../../libs/ulib/include
objs := main.o ../../libs/ulib/ulib.a
.PHONY:all
all: $(objs)
	
	ld -m elf_i386 $(objs) -e main -o test.elf
	cp test.elf ../../../hd/d1
%.o: %.c
	$(CC) $(CFLAGS)  $< -o $@
%.o: %.asm
	$(NASM) -f elf $< -o $@

clean:
	rm $(objs)
	rm test.elf