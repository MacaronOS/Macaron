AS=nasm
CPPC=i686-elf-g++
CC=i686-elf-gcc
ASFLAGS=-felf
LDFLAGS=-Wl,--gc-sections -ffreestanding -nostdlib -g -T src/linker.ld

start.s.o:
	$(AS) $(ASFLAGS) start.s -o $@

define APP_TEMPLATE
$(1)_SRCS := $$(shell find $(1) -name *.cpp -or -name *.c -or -name *.s)
$(1)_OBJS := $$($(1)_SRCS:%=$$($(1))/%.o)
$(1)_OBJS_CURDIR := $$($(1)_SRCS:%=.$$($(1))/%.o)
$(1)_TARGET := $$(1).mapp

TARGETS += $(1)/$(1).mapp

$(1)/$(1).mapp: $$($(1)_OBJS) start.s.o
	$(CC) start.s.o $$($(1)_OBJS_CURDIR) -Wl,--oformat=binary -Ttext 0 -nostdlib -nostdinc -o $$@ -lgcc

# c source
$$($(1))/%.c.o: %.c
	$(CC) $$(CPPFLAGS) $$(CFLAGS) -c $$< -o .$$@

# assembly
$$($(1))/%.s.o: %.s
	$(AS) $$(ASFLAGS) $$< -o .$$@

# cpp source
$$($(1))/%.cpp.o: %.cpp
	$(CPPC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
endef

include */.info.mk
$(foreach app, $(APPS), $(eval $(call APP_TEMPLATE,$(app))))

apps: $(TARGETS)

clean: 
	find . -type f -name "*.mapp" -exec rm -f {} \;
	find . -type f -name "*.o" -exec rm -f {} \;