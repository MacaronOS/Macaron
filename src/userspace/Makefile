ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
include $(ROOT_DIR)/makefile.config

# -------------------------------USERSPACE LIBRARIES----------------------------------

define LIB_TEMPLATE
$(1)_LIB_RELATIVE := $$(shell realpath --relative-to $$(USERSPACE_SRC_DIR) $(1).a)
$(1)_LIB := $$(USERSPACE_BUILD_DIR)/$$($(1)_LIB_RELATIVE)
USERSPACE_LIBS += $$($(1)_LIB)

$(1)_SRCS := $$(shell find $(1) -name *.cpp -or -name *.c -or -name *.s)
$(1)_RELATIVE_SRCS := $$(shell realpath --relative-to $$(USERSPACE_SRC_DIR) $$($(1)_SRCS))
$(1)_OBJS := $$($(1)_RELATIVE_SRCS:%=$$(USERSPACE_BUILD_DIR)/%.o)

$$($(1)_LIB): $$($(1)_OBJS)
	i686-elf-ar rcs $$($(1)_LIB) $$($(1)_OBJS)

# c source
$$(USERSPACE_BUILD_DIR)/%.c.o: $$(USERSPACE_SRC_DIR)/%.c
	mkdir -p $$(dir $$@)
	$(CC) $$(CPPFLAGS) $$(CFLAGS) -c $$< -o $$@

# assembly
$$(USERSPACE_BUILD_DIR)/%.s.o: $$(USERSPACE_SRC_DIR)/%.s
	mkdir -p $$(dir $$@)
	$(AS) $$(ASFLAGS) $$< -o $$@

# cpp source
$$(USERSPACE_BUILD_DIR)/%.cpp.o: $$(USERSPACE_SRC_DIR)/%.cpp
	mkdir -p $$(dir $$@)
	$(CPPC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
endef

ulibs := $(shell find $(USERSPACE_SRC_DIR)/libs/* -maxdepth 0 -type d ! -path "build")

$(foreach lib, $(ulibs), $(eval $(call LIB_TEMPLATE,$(lib))))

libs: $(USERSPACE_LIBS)

# ------------------------------------------------------------------------------------

# -------------------------------USERSPACE APPS---------------------------------------

LIBRARIES_FOLDER ?= $(USERSPACE_BUILD_DIR)/libs/
LINKER_FLAGS ?= -Wl,--oformat=binary -Ttext 0 -nostdlib -nostdinc 

define APP_TEMPLATE
$(1)_APP_RELATIVE := $$(shell realpath --relative-to $$(USERSPACE_SRC_DIR) $(1).app)
$(1)_APP := $$(USERSPACE_BUILD_DIR)/$$($(1)_APP_RELATIVE)
USERSPACE_APPS += $$($(1)_APP)

$(1)_SRCS := $$(shell find $(1) -name *.cpp -or -name *.c -or -name *.s)
$(1)_RELATIVE_SRCS := $$(shell realpath --relative-to $$(USERSPACE_SRC_DIR) $$($(1)_SRCS))
$(1)_OBJS := $$($(1)_RELATIVE_SRCS:%=$$(USERSPACE_BUILD_DIR)/%.o)

$$($(1)_APP): $$($(1)_OBJS)
	$(CC) $$(LINKER_FLAGS) $$($(1)_OBJS) -lc -L$(LIBRARIES_FOLDER) -o $$@ -lgcc

# c source
$$(USERSPACE_BUILD_DIR)/%.c.o: $$(USERSPACE_SRC_DIR)/%.c
	mkdir -p $$(dir $$@)
	$(CC) $$(CPPFLAGS) $$(CFLAGS) -I$$(USERSPACE_SRC_DIR)/libs -c $$< -o $$@

# assembly
$$(USERSPACE_BUILD_DIR)/%.s.o: %.s
	mkdir -p $$(dir $$@)
	$(AS) $$(ASFLAGS) $$< -o $$@

# cpp source
$$(USERSPACE_BUILD_DIR)/$(1)/%.cpp.o: %.cpp
	mkdir -p $$(dir $$@)
	$(CPPC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
endef

uapps := $(shell find $(USERSPACE_SRC_DIR)/apps/* -maxdepth 0 -type d ! -path "build")
$(foreach app, $(uapps), $(eval $(call APP_TEMPLATE,$(app))))

apps: $(USERSPACE_LIBS) $(USERSPACE_APPS)
	echo $(USERSPACE_APPS)

# ------------------------------------------------------------------------------------

clean_userspace:
	$(RM) -r $(USERSPACE_BUILD_DIR)
