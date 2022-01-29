# Copyright 2018 Patrick Dowling.
#
# Author: Patrick Dowling (pld@gurkenkiste.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# Expected to be defined are
#
# PROJECT = <name of project>
# PROJECT_SRC_DIRS = <directories containing project source>
# PROJECT_RESOURCE_DIR = <directory with python buildable resources>
# PROJECT_RESOURCE_SCRIPT = <main project python resource script>
# RAM_SIZE = <size of ram in K>
# FLASH_SIZE = <size of flash in K>
# F_CPU
#
# Optional defines
# PROJECT_DEFINES = <additional project-specific defines (without -D)>
# FLASH_SETTINGS_SIZE (optional)
# PROJECT_INCLUDE_DIRS

check_variable_list = \
    $(strip $(foreach 1,$1, \
        $(call __check_variable_list,$1,$(strip $(value 2)))))
__check_variable_list = \
    $(if $(value $1),, \
      $(error Undefined $1$(if $2, ($2))))

$(call check_variable_list, PROJECT PROJECT_SRC_DIRS)
$(call check_variable_list, MODEL, F_CPU)
$(call check_variable_list, RAM_SIZE FLASH_SIZE)

###
## General setup
#
TOOLCHAIN_PATH ?= /usr/local/arm-4.8.3/
BUILD_DIR	?= build/$(PROJECT)/
OPTIMIZE ?= -O2
FLASH_TARGET ?= flash_bmp

STM32X_DIR = stm32x/
SCRIPT_DIR = $(STM32X_DIR)scripts/

STM32X_CPPSTD ?= c++11
STM32X_CSTD   ?= c11

PROJECT_SRC_DIRS += $(PROJECT_RESOURCE_DIR)
PROJECT_RESOURCE_FILE = $(PROJECT_RESOURCE_SCRIPT:.py=.cc)

PROJECT_SRC_DIRS += $(STM32X_DIR)util $(STM32X_DIR)stm32x

SYSTEM_DEFINES += $(MODEL)
SYSTEM_DEFINES += F_CPU=$(F_CPU)
SYSTEM_DEFINES += USE_STDPERIPH_DRIVER

MAX_FRAME_SIZE ?= 128

C_FLAGS += -g -Wall -Werror -Wextra -Wshadow \
	   -fasm \
	   -finline \
	   -finline-functions-called-once \
	   -fdata-sections -ffunction-sections \
	   -fshort-enums \
	   -fno-move-loop-invariants \
	   -Wlogical-op \
	   -Wduplicated-branches \
	   -Wduplicated-cond \
	   -Wframe-larger-than=$(MAX_FRAME_SIZE) \
	   -Wdouble-promotion

CPP_FLAGS += -fno-exceptions \
	     -fno-rtti -fno-use-cxa-atexit \
	     -Wpedantic \
	     -Wnon-virtual-dtor \
	     -Woverloaded-virtual

# -Wdouble-promotion -> printf
# -Wconversion -> OMGWTFBBQ
# -Wold-style-cast -> StdPeriphLib :(

ARCH_FLAGS += -mthumb -mthumb-interwork -funroll-loops -specs=nano.specs -specs=nosys.specs

###
## Model-specific handling
#
ifneq (,$(findstring STM32F0,$(MODEL)))
MODEL_INC = $(STM32X_DIR)makefile.F0xx.inc
else ifneq (,$(findstring STM32F37,$(MODEL)))
MODEL_INC = $(STM32X_DIR)makefile.F37x.inc
else ifneq (,$(findstring STM32F4,$(MODEL)))
MODEL_INC = $(STM32X_DIR)makefile.F4xx.inc
endif
ifeq (,$(MODEL_INC))
MODEL_INC = $(error Undefined model '$(MODEL)')
endif

STM32X_EXTERN_DIR = $(STM32X_DIR)extern/
STM32X_CMSIS_DIR = $(STM32X_EXTERN_DIR)CMSIS
ST_DIR = $(STM32X_DIR)extern/ST/

include $(MODEL_INC)

###
## Source & object files
#
INCLUDES += $(PROJECT_INCLUDE_DIRS)
INCLUDES += $(STM32X_DIR)
INCLUDES += $(STM32X_CMSIS_DIR)/Core/Include
INCLUDES += $(STM32X_CMSIS_DIR)/DSP/Include

C_FILES   += $(notdir $(wildcard $(patsubst %,%/*.c,$(PROJECT_SRC_DIRS))))
CC_FILES  += $(notdir $(wildcard $(patsubst %,%/*.cc,$(PROJECT_SRC_DIRS))))
AS_FILES  += $(notdir $(wildcard $(patsubst %,%/*.s,$(PROJECT_SRC_DIRS))))

VPATH += $(PROJECT_SRC_DIRS)
OBJDIR = $(BUILD_DIR)

RESOURCE_PY_FILES = $(filter-out $(PROJECT_RESOURCE_SCRIPT), $(wildcard $(PROJECT_RESOURCE_DIR)[!_]*.py))

###
## System core and linker
#
SYSTEM_DEFINES += \
	RAM_SIZE=$(shell $(NUMFMT) --from=iec $(RAM_SIZE)) \
	FLASH_SIZE=$(shell $(NUMFMT) --from=iec $(FLASH_SIZE))

ifneq (,$(MIN_STACK_SIZE))
SYSTEM_DEFINES += \
	MIN_STACK_SIZE=$(shell $(NUMFMT) --from=iec $(MIN_STACK_SIZE))
endif

ifneq (,$(FLASH_SETTINGS_SIZE))
SYSTEM_DEFINES += \
	FLASH_SETTINGS_SIZE=$(shell $(NUMFMT) --from=iec $(FLASH_SETTINGS_SIZE))
endif

ifeq ($(ENABLE_LIBC_INIT_ARRAY),TRUE)
	SYSTEM_DEFINES += ENABLE_LIBC_INIT_ARRAY
endif

ifneq (,$(HSE_VALUE))
SYSTEM_DEFINES += \
	HSE_VALUE=$(HSE_VALUE)
endif

PROJECT_LINKER_SCRIPT = $(BUILD_DIR)$(PROJECT).ld

LD_FLAGS = \
			-Wl,-Map=$(MAPFILE) \
			-Wl,--gc-sections \
			-T $(PROJECT_LINKER_SCRIPT) \
			$(ARCH_FLAGS) \
			$(addprefix -I, $(INCLUDES)) \
			$(addprefix -D, $(SYSTEM_DEFINES)) \
			-L$(BUILD_DIR) \
			$(PROJECT_LINKER_FLAGS)

###
## Setup
#
ifdef VERBOSE
Q :=
ECHO := @true
else
Q := @
ECHO := @echo
endif

ARCH_PATH = $(TOOLCHAIN_PATH)bin/arm-none-eabi
CC				= $(ARCH_PATH)-gcc
CXX				= $(ARCH_PATH)-g++
AS				= $(ARCH_PATH)-as
OBJCOPY		= $(ARCH_PATH)-objcopy
GDB				= $(ARCH_PATH)-gdb
OBJDUMP		= $(ARCH_PATH)-objdump
AR				= $(ARCH_PATH)-ar
SIZE			= $(ARCH_PATH)-size
NM				= $(ARCH_PATH)-nm
RM				= rm -f
MKDIR			= mkdir -p
NUMFMT	:= $(shell command -v numfmt 2> /dev/null)
ifeq (,$(NUMFMT))
	NUMFMT  := gnumfmt
endif


###
## Objects & build rules
#
BINFILE = $(BUILD_DIR)$(PROJECT).bin
ELFFILE = $(BUILD_DIR)$(PROJECT).elf
HEXFILE = $(BUILD_DIR)$(PROJECT).hex
MAPFILE = $(BUILD_DIR)$(PROJECT).map
SIZEFILE = $(BUILD_DIR)$(PROJECT).size
DISFILE  = $(BUILD_DIR)$(PROJECT).s

OBJS  = $(patsubst %,$(OBJDIR)%,$(C_FILES:.c=.o))
OBJS += $(patsubst %,$(OBJDIR)%,$(CC_FILES:.cc=.o))
OBJS += $(patsubst %,$(OBJDIR)%,$(AS_FILES:.s=.o))
DEPS  = $(OBJS:.o=.d)

C_FLAGS += $(OPTIMIZE)
C_FLAGS += $(addprefix -I, $(INCLUDES))
C_FLAGS += $(addprefix -D, $(PROJECT_DEFINES)) $(addprefix -D, $(SYSTEM_DEFINES)) $(ARCH_FLAGS)
C_FLAGS += -MMD -MP # dependency generation

$(BUILD_DIR)%.o: %.c
	$(ECHO) "C $<..."
	$(Q)$(CC) -c -std=$(STM32X_CSTD) $(C_FLAGS) $< -o $@

$(BUILD_DIR)%.o: %.cc
	$(ECHO) "CC $<..."
	$(Q)$(CXX) -c -std=$(STM32X_CPPSTD) $(C_FLAGS) $(CPP_FLAGS) $< -o $@

$(BUILD_DIR)%.o: %.s
	$(ECHO) "AS $<..."
	$(Q)$(CC) -c $(addprefix -D, $(SYSTEM_DEFINES)) -x assembler-with-cpp $< -o $@

$(BUILD_DIR)%.hex: $(BUILD_DIR)%.elf
	$(ECHO) "Creating hex $@..."
	$(Q)$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)%.bin: $(BUILD_DIR)%.elf
	$(ECHO) "Creating binary $@..."
	$(Q)$(OBJCOPY) -O binary $< $@

$(ELFFILE): $(BUILD_DIR) $(OBJS) $(PROJECT_LINKER_SCRIPT)
	$(ECHO) "Linking $@..."
	$(Q)$(CC) $(LD_FLAGS) -o $(ELFFILE) $(OBJS)

$(PROJECT_LINKER_SCRIPT): $(LINKER_SCRIPT_IN)
	$(ECHO) "Generating linker script from $(LINKER_SCRIPT_IN)..."
	$(Q)$(CC) $(addprefix -D, $(SYSTEM_DEFINES)) -E -P -x c-header $< -o $@

$(SIZEFILE): $(ELFFILE)
	$(Q)$(SIZE) $(ELFFILE) > $(SIZEFILE)

$(DISFILE): $(ELFFILE)
	$(ECHO) "DIS $@..."
	$(Q)$(OBJDUMP) -dC -h -S $< > $@

###
## Targets
#
.PHONY: all
all: $(BUILD_DIR) $(HEXFILE)

$(BUILD_DIR):
	@$(MKDIR) $(BUILD_DIR)

.PHONY: bin
bin: $(BINFILE)

.PHONY: hex
hex: $(HEXFILE)

.PHONY: clean
clean:
	@$(RM) $(OBJS) $(DEPS)
	@$(RM) $(ELFFILE) $(BINFILE) $(HEXFILE) $(MAPFILE) $(PROJECT_LINKER_SCRIPT) $(SIZEFILE)

.PHONY: size
size: $(SIZEFILE)
	@cat $(SIZEFILE)

.PHONY: disassemble
disassemble: $(DISFILE)

.PHONY: resources
resources: $(PROJECT_RESOURCE_FILE)

$(PROJECT_RESOURCE_SCRIPT:.py=.cc): $(PROJECT_RESOURCE_SCRIPT) $(RESOURCE_PY_FILES)
	$(ECHO) "PY $^"
	$(Q)PYTHONPATH="$(STM32X_DIR)" python3 $(PROJECT_RESOURCE_SCRIPT) $(PROJECT_RESOURCE_FILE)

-include $(DEPS)

###
## Flash/programming
#

.PHONY: flash
flash: $(FLASH_TARGET)

ifeq ($(FLASH_TARGET),flash_bmp)
ifeq ($(BMP_PORT),)
BMP_PORT_CANDIDATES := $(wildcard /dev/cu.usbmodem*1)
ifeq ($(words $(BMP_PORT_CANDIDATES)),1)
BMP_PORT := $(BMP_PORT_CANDIDATES)
else
BMP_PORT = $(error Black Magic Probe gdb serial port not found, please provide the device name via the BMP_PORT variable parameter$(if \
$(BMP_PORT_CANDIDATES), (found $(BMP_PORT_CANDIDATES))))
endif
endif
endif

.PHONY: flash_bmp
flash_bmp: $(ELFFILE)
	$(Q)$(GDB) -nx -batch \
						-ex 'target extended-remote $(BMP_PORT)' \
						-x $(SCRIPT_DIR)bmp_flash_swd.scr \
						$(ELFFILE)

.PHONY: debug_bmp
debug_bmp: $(ELFFILE)
	$(Q)$(GDB) -ex 'target extended-remote $(BMP_PORT)' \
						-x $(SCRIPT_DIR)bmp_gdb_swd.scr \
						$(ELFFILE)
