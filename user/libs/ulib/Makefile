NASM := nasm
CC   := gcc
LD   := ld



CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
             -nostartfiles -nodefaultlibs  -c 
objs:= sys/syscall.o stdio.o
.PHONY:all
all: $(objs) 
	ar rcs ulib.a $(objs)
%.o: %.c
	$(CC) $(CFLAGS)  $< -o $@
%.o: %.asm
	$(NASM) -f elf $< -o $@

clean:
	rm $(objs)
	rm ulib.a