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
CPPC=i686-elf-g++
CC=i686-elf-gcc
ASFLAGS=-felf
LDFLAGS=-Wl,--gc-sections -ffreestanding -nostdlib -g -T src/linker.ld

DISK=drive.img
QEMUFLAGS=-device piix3-ide,id=ide -drive id=disk,file=${DISK},if=none -device ide-drive,drive=disk,bus=ide.0

TARGET_EXEC ?= a.out

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS_KERNEL:=$(shell find ./src -type f -name "*.s" ! -path "./src/userspace/*")
SRCS_KERNEL+=$(shell find ./src -type f -name "*.cpp" ! -path "./src/userspace/*")
OBJS_KERNEL:=$(SRCS_KERNEL:%=$(BUILD_DIR)/%.o)

DEPS := $(OBJS_KERNEL:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS ?= $(INC_FLAGS)-nostdlib -nostdinc -fno-builtin -fno-stack-protector -ffreestanding -fno-exceptions -m32 -Iinclude -fno-use-cxa-atexit -fno-rtti -fno-leading-underscore -Wno-write-strings

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS_KERNEL)
	$(CC) $(LDFLAGS) $(OBJS_KERNEL) -o $@ -lgcc

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# cpp source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CPPC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)
	$(MAKE) clean -C src/userspace

drive:
	rm -f ${DISK}
	qemu-img create -f raw ${DISK} 16M
	sudo ${EXT2_FORMATTER} -t ext2 -r 0 -b 1024 ${DISK}

	sudo mkdir -p mountpoint
	sudo ${MOUNT_EXT2} ${DISK} mountpoint -o rw+
	sudo touch mountpoint/file.txt
	sudo bash -c 'echo "testing..." > mountpoint/file.txt'
	$(MAKE) apps -C src/userspace
	sudo mkdir -p mountpoint/apps/
	sudo find ./src/userspace -type f -name "*.mapp" -exec cp {} ./mountpoint/apps \;
	sudo umount mountpoint

run:
	qemu-system-i386 $(QEMUFLAGS) -kernel $(BUILD_DIR)/a.out

-include $(DEPS)

MKDIR_P ?= mkdir -p