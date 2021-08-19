ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
include $(ROOT_DIR)/makefile.config

# -------------------------------WISTERIALIB-----------------------------------------

SRCS_WISTERIALIB:=$(shell find $(WISTERIALIB_SRC_DIR)/* -type f -name *.cpp -or -name *.c -or -name *.s)
RELATIVE_SRCS_WISTERIALIB:=$(shell realpath --relative-to $(WISTERIALIB_SRC_DIR) $(SRCS_WISTERIALIB))
OBJS_WISTERIALIB:=$(RELATIVE_SRCS_WISTERIALIB:%=$(WISTERIALIB_BUILD_DIR)/%.o)

$(WISTERIALIB_BUILD_DIR)/$(WISTERIALIB): $(OBJS_WISTERIALIB)
	$(AR) rcs $(WISTERIALIB_BUILD_DIR)/$(WISTERIALIB) $(OBJS_WISTERIALIB)

# assembly
$(WISTERIALIB_BUILD_DIR)/%.s.o: $(WISTERIALIB_SRC_DIR)/%.s
	$(MKDIR_P) $(dir $@)
	@ $(AS) $(ASFLAGS) $< -o $@

# cpp source
$(WISTERIALIB_BUILD_DIR)/%.cpp.o: $(WISTERIALIB_SRC_DIR)/%.cpp
	$(MKDIR_P) $(dir $@)
	$(CPPC) $(CFLAGS) $(CPPFLAGS) -I$(WISTERIALIB_SRC_DIR)/include/Wisterialib -DWisteriaLibBuild -ggdb -c $< -o $@

Wisterialib: $(WISTERIALIB_BUILD_DIR)/$(WISTERIALIB)

# ------------------------------------------------------------------------------------

clean_Wisterialib:
	$(RM) -r $(WISTERIALIB_BUILD_DIR)
