EXT2_FORMATTER 				:=
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	EXT2_FORMATTER = mkfs
endif
ifeq ($(UNAME_S), Darwin)
	EXT2_FORMATTER = /usr/local/opt/e2fsprogs/sbin/mkfs.ext2
endif

MOUNT_EXT2=fuse-ext2

AS=nasm
CC=i686-elf-gcc
ASFLAGS=-felf
LDFLAGS=-ffreestanding -nostdlib -g -T src/linker.ld

DISC=drive.img
QEMUFLAGS=-device piix3-ide,id=ide -drive id=disk,file=${DISC},if=none -device ide-drive,drive=disk,bus=ide.0

TARGET_EXEC ?= a.out

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS ?= $(INC_FLAGS)-nostdlib -nostdinc -fno-builtin -fno-stack-protector -ffreestanding

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@ -lgcc

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

drive:
	rm -f ${DISC}
	qemu-img create -f raw ${DISC} 16M
	sudo ${EXT2_FORMATTER} -t ext2 -r 0 -b 1024 ${DISC}

	sudo mkdir -p mountpoint
	sudo ${MOUNT_EXT2} ${DISC} mountpoint -o rw+
	sudo touch mountpoint/file.txt
	sudo bash -c 'echo "testing..." > mountpoint/file.txt'
	sudo umount mountpoint

run:
	qemu-system-i386 $(QEMUFLAGS) -kernel $(BUILD_DIR)/a.out

-include $(DEPS)

MKDIR_P ?= mkdir -p