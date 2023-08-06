OBJS = framebuffer.o life.o mailbox.o timer.o uart.o kernel.o
LLVMPATH = /opt/homebrew/opt/llvm/bin
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	CC = $(LLVMPATH)/clang
	LD = $(LLVMPATH)/ld.lld -m aarch64elf
	OBJCOPY = $(LLVMPATH)/llvm-objcopy
endif
ifeq ($(UNAME), Linux)
	CC = clang
	LD = ld.lld -m aarch64elf
	OBJCOPY = llvm-objcopy
endif

CFLAGS = --target=aarch64-elf -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd

all: clean kernel8.img testuart.img testframebuffer.img

boot.o: boot.s
	$(CC) --target=aarch64-elf -Wall -O2 -nostdlib -mcpu=cortex-a53+nosimd -c boot.s -o boot.o
framebuffer.o: framebuffer.h mailbox.h uart.h
life.o: life.h framebuffer.h uart.h
mailbox.o: mailbox.h
timer.o: timer.h
uart.o: uart.h
kernel.o: framebuffer.h life.h mailbox.h timer.h uart.h
testuart.o: uart.h
testframebuffer.o: framebuffer.h mailbox.h uart.h
testlife.o: framebuffer.h life.h mailbox.h uart.h

kernel8.img: boot.o $(OBJS)
	$(LD)  -nostdlib boot.o $(OBJS) -T link.ld -o kernel8.elf
	$(OBJCOPY) -O binary kernel8.elf kernel8.img

testuart.img: boot.o testuart.o uart.o
	$(LD)  -nostdlib boot.o testuart.o uart.o -T link.ld -o testuart.elf
	$(OBJCOPY) -O binary testuart.elf testuart.img

testframebuffer.img: boot.o testframebuffer.o framebuffer.o mailbox.o uart.o
	$(LD)  -nostdlib boot.o testframebuffer.o framebuffer.o mailbox.o uart.o -T link.ld -o testframebuffer.elf
	$(OBJCOPY) -O binary testframebuffer.elf testframebuffer.img

testlife.img: boot.o testlife.o framebuffer.o mailbox.o uart.o life.o
	$(LD)  -nostdlib boot.o testlife.o framebuffer.o mailbox.o uart.o life.o -T link.ld -o testlife.elf
	$(OBJCOPY) -O binary testlife.elf testlife.img

clean:
	$(RM) *.img *.elf *.o

run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio

test_uart: testuart.img
	qemu-system-aarch64 -M raspi3b -kernel testuart.img -serial null -serial stdio

test_framebuffer: testframebuffer.img
	qemu-system-aarch64 -M raspi3b -kernel testframebuffer.img -monitor stdio

test_life: testlife.img
	qemu-system-aarch64 -M raspi3b -kernel testlife.img -monitor stdio
