include makefile.config
include src/kernel/Makefile
include src/userspace/Makefile

run: build
	$(QEMU) $(QEMUFLAGS) -kernel $(KERNEL_BUILD_DIR)/$(TARGET_EXEC)

debug:
	$(QEMU) -s -S -snapshot $(QEMUFLAGS) -kernel $(KERNEL_BUILD_DIR)/$(TARGET_EXEC)

build: apps kernel

iso: kernel
	mkdir -p $(KERNEL_BUILD_DIR)/iso/boot/grub
	cp $(KERNEL_BUILD_DIR)/$(TARGET_EXEC) $(KERNEL_BUILD_DIR)/iso/boot
	cp grub.cfg $(KERNEL_BUILD_DIR)/iso/boot/grub/grub.cfg
	rm -f $(KERNEL_BUILD_DIR)/iso/$(TARGET_EXEC_ISO)
	grub-mkrescue -o $(KERNEL_BUILD_DIR)/iso/$(TARGET_EXEC_ISO) $(KERNEL_BUILD_DIR)/iso

install:
	sudo ${MOUNT_EXT2} ${DISK} mountpoint -o rw+
	sudo $(MKDIR_P) mountpoint/apps/
	sudo find $(USERSPACE_BUILD_DIR)/apps/* -type f -name "*.app" -exec cp {} ./mountpoint/apps/ \;
	sudo umount mountpoint
	
drive:
	$(RM) -f ${DISK}
	$(QEMU_IMG) create -f raw ${DISK} 16M
	sudo ${EXT2_FORMATTER} -t ext2 -r 0 -b 1024 ${DISK}

	sudo $(MKDIR_P) mountpoint
	sudo ${MOUNT_EXT2} ${DISK} mountpoint -o rw+
	sudo touch mountpoint/file.txt
	sudo bash -c 'echo "testing..." > mountpoint/file.txt'
	sudo umount mountpoint

build_test: kernel_tester

test: build_test
	qemu-system-i386 $(QEMUFLAGS) -kernel $(KERNEL_BUILD_DIR)/$(TEST_EXEC)

clean:
	$(RM) -r $(BUILD_DIR)