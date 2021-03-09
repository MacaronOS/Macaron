ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
include $(ROOT_DIR)/makefile.config

# -------------------------------BUILDING KERNEL---------------------------------------

SRCS_KERNEL:=$(shell find $(KERNEL_SRC_DIR)/* -type f -name *.cpp -or -name *.c -or -name *.s)
RELATIVE_SRCS_KERNEL:=$(shell realpath --relative-to $(KERNEL_SRC_DIR) $(SRCS_KERNEL))
OBJS_KERNEL:=$(RELATIVE_SRCS_KERNEL:%=$(KERNEL_BUILD_DIR)/%.o)

$(KERNEL_BUILD_DIR)/$(TARGET_EXEC): $(OBJS_KERNEL)
	$(CC) $(LDFLAGS) -T $(KERNEL_SRC_DIR)/linker.ld $(OBJS_KERNEL) -o $@ -lgcc

# assembly
$(KERNEL_BUILD_DIR)/%.s.o: $(KERNEL_SRC_DIR)/%.s
	$(MKDIR_P) $(dir $@)
	@ $(AS) $(ASFLAGS) $< -o $@

# cpp source
$(KERNEL_BUILD_DIR)/%.cpp.o: $(KERNEL_SRC_DIR)/%.cpp
	$(MKDIR_P) $(dir $@)
	$(CPPC) $(CPPFLAGS) -ggdb $(CFLAGS) -I$(KERNEL_SRC_DIR)/src/ -c $< -o $@

kernel:	$(KERNEL_BUILD_DIR)/$(TARGET_EXEC)

# -------------------------------------------------------------------------------------

# -------------------------------BUILDING TESTER---------------------------------------

SRCS_KERNEL:=$(shell find $(KERNEL_SRC_DIR)/* -type f -name *.cpp -or -name *.c -or -name *.s)
RELATIVE_SRCS_KERNEL:=$(shell realpath --relative-to $(KERNEL_SRC_DIR) $(SRCS_KERNEL))
OBJS_TESTER:=$(RELATIVE_SRCS_KERNEL:%=$(KERNEL_BUILD_DIR)/%.test.o)

$(KERNEL_BUILD_DIR)/$(TEST_EXEC): $(OBJS_TESTER)
	$(CC) $(LDFLAGS) -T $(KERNEL_SRC_DIR)/linker.ld $(OBJS_TESTER) -o $@ -lgcc

$(KERNEL_BUILD_DIR)/%.s.test.o: $(KERNEL_SRC_DIR)/%.s
	$(MKDIR_P) $(dir $@)
	$(AS) -DHinliXX_TEST $(ASFLAGS) $< -o $@

$(KERNEL_BUILD_DIR)/%.cpp.test.o: $(KERNEL_SRC_DIR)/%.cpp
	$(MKDIR_P) $(dir $@)
	$(CPPC) $(CPPFLAGS) -DHinliXX_TEST $(CFLAGS) -I$(KERNEL_SRC_DIR)/src/ -c $< -o $@

kernel_tester: $(KERNEL_BUILD_DIR)/$(TEST_EXEC)

# -------------------------------------------------------------------------------------

clean_kernel:
	$(RM) -r $(KERNEL_BUILD_DIR)