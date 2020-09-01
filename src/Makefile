SOURCES=boot.o kernel.o

CFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS=linker.ld
ASFLAGS=-felf

all: $(SOURCES) link 

build:
	i686-elf-gcc $(CFLAGS) -c kernel/kernel.c -o kernel.o
	nasm $(ASFLAGS) boot.s
	i686-elf-gcc -ffreestanding -nostdlib -g -T linker.ld $(SOURCES) -o mykernel.elf -lgcc

run:
	qemu-system-i386 -kernel mykernel.elf

clean:
	rm *.o mykernel.elf

link:
	ld $(LDFLAGS) -o kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<
