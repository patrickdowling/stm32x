# Copyright 2018-2024 Patrick Dowling.
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
# Expected to be defined are:
# TOOLCHAIN_PATH = root path for compiler (gcc)
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
#

STM32X_DIR = ./stm32x
include $(STM32X_DIR)/mk/utils.mk

$(call check_variable_list, TOOLCHAIN_PATH)
$(call check_variable_list, PROJECT PROJECT_SRC_DIRS)
$(call check_variable_list, MODEL F_CPU HSE_VALUE)
$(call check_variable_list, RAM_SIZE FLASH_SIZE)

###
## General setup
#
BUILD_DIR ?= build/$(PROJECT)

OPTIMIZE ?= -O2
FLASH_TARGET ?= flash_bmp

SCRIPT_DIR = $(STM32X_DIR)/scripts

STM32X_CPPSTD ?= c++17
STM32X_CSTD   ?= c11

PROJECT_SRC_DIRS += $(PROJECT_RESOURCE_DIR)
PROJECT_RESOURCE_FILE = $(PROJECT_RESOURCE_SCRIPT:.py=.cc)

PROJECT_SRC_DIRS += $(STM32X_DIR)/src
PROJECT_SRC_DIRS += $(STM32X_DIR)/src/util

SYSTEM_DEFINES += F_CPU=$(F_CPU)

MAX_FRAME_SIZE ?= 128

ifeq (TRUE,$(DISABLE_WDOUBLE_PROMOTION))
	OPTIONAL_C_FLAGS += -Wno-double-promotion
else
	OPTIONAL_C_FLAGS += -Wdouble-promotion
endif

C_FLAGS += -g -Wall -Werror -Wextra \
	   -fasm \
	   -finline \
	   -finline-functions-called-once \
	   -fdata-sections -ffunction-sections \
	   -fshort-enums \
	   -fno-move-loop-invariants \
	   -fdevirtualize \
	   -Wcast-align=strict \
	   -Wduplicated-branches \
	   -Wduplicated-cond \
	   -Wframe-larger-than=$(MAX_FRAME_SIZE) \
	   -Wlogical-op \
	   -Wredundant-decls \
	   -Wshadow \
	   -Wshift-overflow=2 \
	   -Wtrampolines \
	   -Wundef

CPP_FLAGS += -fno-exceptions \
	     -fno-rtti -fno-use-cxa-atexit \
	     -Wpedantic \
	     -Wctor-dtor-privacy \
	     -Wextra-semi \
	     -Wnoexcept \
	     -Wnon-virtual-dtor \
	     -Wzero-as-null-pointer-constant \
	     -Woverloaded-virtual \
	     -Wzero-as-null-pointer-constant

C_FLAGS += $(OPTIONAL_C_FLAGS)
CPP_FLAGS += $(OPTIONAL_CPP_FLAGS)

# -Wuseless-cast -> CMSIS et al
# -Wdouble-promotion -> printf
# -Wconversion -> OMGWTFBBQ
# -Wold-style-cast -> StdPeriphLib :(

ARCH_FLAGS += -mthumb -mthumb-interwork -funroll-loops -specs=nano.specs -specs=nosys.specs

###
## Model-specific handling
#
ifneq (,$(findstring STM32F0,$(MODEL)))
MODEL_INC = $(STM32X_DIR)/makefile.F0xx.inc
else ifneq (,$(findstring STM32F37,$(MODEL)))
MODEL_INC = $(STM32X_DIR)/makefile.F37x.inc
else ifneq (,$(findstring STM32F4,$(MODEL)))
MODEL_INC = $(STM32X_DIR)/makefile.F4xx.inc
endif
ifeq (,$(MODEL_INC))
MODEL_INC = $(error Undefined model '$(MODEL)')
endif

STM32X_EXTERN_DIR = $(STM32X_DIR)/extern/
STM32X_CMSIS_DIR = $(STM32X_EXTERN_DIR)CMSIS
ST_DIR = $(STM32X_DIR)/extern/ST/

SYSTEM_DEFINES += $(MODEL)
SYSTEM_DEFINES += USE_STDPERIPH_DRIVER

INCLUDES += $(STM32X_CMSIS_DIR)/Core/Include
INCLUDES += $(STM32X_CMSIS_DIR)/DSP/Include

include $(MODEL_INC)

###
## Source & object files
#
INCLUDES += $(PROJECT_INCLUDE_DIRS)
INCLUDES += $(PROJECT_RESOURCE_DIR)
INCLUDES += $(STM32X_DIR)/include

C_FILES   += $(notdir $(wildcard $(patsubst %,%/*.c,$(PROJECT_SRC_DIRS))))
CC_FILES  += $(notdir $(wildcard $(patsubst %,%/*.cc,$(PROJECT_SRC_DIRS))))
AS_FILES  += $(notdir $(wildcard $(patsubst %,%/*.s,$(PROJECT_SRC_DIRS))))

VPATH += $(PROJECT_SRC_DIRS)
OBJDIR = $(BUILD_DIR)

RESOURCE_PY_FILES = $(filter-out $(PROJECT_RESOURCE_SCRIPT), $(wildcard $(PROJECT_RESOURCE_DIR)/[!_]*.py))

###
## System core and linker
#
SYSTEM_DEFINES += \
	RAM_SIZE=$(shell $(NUMFMT) --from=iec $(RAM_SIZE)) \
	FLASH_SIZE=$(shell $(NUMFMT) --from=iec $(FLASH_SIZE))

ifneq (,$(FLASH_ORIGIN))
SYSTEM_DEFINES += FLASH_ORIGIN=$(FLASH_ORIGIN)
endif

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

PROJECT_LINKER_SCRIPT = $(BUILD_DIR)/$(PROJECT).ld

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
ARCH_PATH = $(TOOLCHAIN_PATH)/arm-none-eabi
CC	:= $(ARCH_PATH)-gcc
CXX	:= $(ARCH_PATH)-g++
AS	:= $(ARCH_PATH)-as
OBJCOPY	:= $(ARCH_PATH)-objcopy
GDB	:= $(ARCH_PATH)-gdb
OBJDUMP	:= $(ARCH_PATH)-objdump
AR	:= $(ARCH_PATH)-ar
SIZE	:= $(ARCH_PATH)-size
NM	:= $(ARCH_PATH)-nm
LD	:= $(ARCH_PATH)-ld
RM	:= rm -f
MKDIR	:= mkdir -p
NUMFMT	:= $(shell command -v numfmt 2> /dev/null)
ifeq (,$(NUMFMT))
	NUMFMT  := gnumfmt
endif


###
## Objects & build rules
#
BINFILE = $(BUILD_DIR)/$(PROJECT).bin
ELFFILE = $(BUILD_DIR)/$(PROJECT).elf
HEXFILE = $(BUILD_DIR)/$(PROJECT).hex
MAPFILE = $(BUILD_DIR)/$(PROJECT).map
SIZEFILE = $(BUILD_DIR)/$(PROJECT).size
DISFILE  = $(BUILD_DIR)/$(PROJECT).s

OBJS = $(patsubst %,$(OBJDIR)/%,$(C_FILES:.c=.o))
OBJS += $(patsubst %,$(OBJDIR)/%,$(CC_FILES:.cc=.o))
OBJS += $(patsubst %,$(OBJDIR)/%,$(AS_FILES:.s=.o))
DEPS  = $(OBJS:.o=.d)

C_FLAGS += $(OPTIMIZE)
C_FLAGS += $(addprefix -I, $(INCLUDES))
C_FLAGS += $(addprefix -D, $(PROJECT_DEFINES)) $(addprefix -D, $(SYSTEM_DEFINES)) $(ARCH_FLAGS)
C_FLAGS += -MMD -MP # dependency generation

$(BUILD_DIR)/%.o: %.c
	$(ECHO) "CC $<..."
	$(Q)$(CC) -c -std=$(STM32X_CSTD) $(C_FLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.cc
	$(ECHO) "CXX $<..."
	$(Q)$(CXX) -c -std=$(STM32X_CPPSTD) $(C_FLAGS) $(CPP_FLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s
	$(ECHO) "AS $<..."
	$(Q)$(CC) -c $(addprefix -D, $(SYSTEM_DEFINES)) -x assembler-with-cpp $< -o $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(ECHO) "Creating hex $@..."
	$(Q)$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(ECHO) "Creating binary $@..."
	$(Q)$(OBJCOPY) -O binary $< $@

$(ELFFILE): $(BUILD_DIR) $(OBJS) $(EXTRA_OBJS) $(PROJECT_LINKER_SCRIPT) Makefile
	$(ECHO) "Linking $@..."
	$(Q)$(CC) $(LD_FLAGS) -o $(ELFFILE) $(OBJS) $(EXTRA_OBJS)

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
	@$(RM) $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d
	@$(RM) $(ELFFILE) $(BINFILE) $(HEXFILE) $(MAPFILE) $(PROJECT_LINKER_SCRIPT) $(SIZEFILE)
	@$(RM) $(PINOUT_SCRIPT) $(BUILD_DIR)/*.log

.PHONY: size
size: $(SIZEFILE)
	@cat $(SIZEFILE)

.PHONY: disassemble
disassemble: $(DISFILE)

.PHONY: resources
resources: $(PROJECT_RESOURCE_FILE)

$(PROJECT_RESOURCE_FILE): $(RESOURCE_PY_FILES)

$(PROJECT_RESOURCE_SCRIPT:.py=.cc): $(PROJECT_RESOURCE_SCRIPT) $(RESOURCE_PY_FILES)
	$(ECHO) "PY $^"
	$(Q)PYTHONPATH="$(STM32X_DIR)/" python3 $(PROJECT_RESOURCE_SCRIPT) $(PROJECT_RESOURCE_FILE)


.PHONY: dump
dump:
	@echo "PROJECT_RESOURCE_DIR=$(PROJECT_RESOURCE_DIR)"
	@echo "RESOURCE_PY_FILES=$(RESOURCE_PY_FILES)"
	@echo "OBJS=$(OBJS)"
	@echo "EXTRA_OBJS=$(EXTRA_OBJS)"

-include $(DEPS)

###
## GPIO export (still super experimental)
#
PROJECT_CSV_FILE = $(PROJECT_IOC_FILE:.ioc=.csv)
PINOUT_SCRIPT = $(BUILD_DIR)/$(PROJECT).scr
PINOUT_GPIO_PATH ?= ./src/drivers/$(PROJECT)_gpio

ifeq "$(shell uname)" "Linux"
CUBEMX_ROOT ?= $(shell realpath ~)/STM32CubeMX
CUBEMX_EXE = $(CUBEMX_ROOT)/STM32CubeMX
CUBEMX = $(CUBEMX_ROOT)
else
CUBEMX_ROOT ?= /Applications/STMicroelectronics/STM32CubeMX.app
CUBEMX_EXE = $(CUBEMX_ROOT)/Contents/MacOs/STM32CubeMX
CUBEMX = $(CUBEMX_ROOT)/Contents/Resources
endif

$(PINOUT_SCRIPT): $(BUILD_DIR) $(PROJECT_IOC_FILE)
	$(ECHO) "Building CubeMX script $@"
	@echo "config load $(shell realpath $(PROJECT_IOC_FILE))" > $@
	@echo "csv pinout $(shell realpath $(PROJECT_CSV_FILE))" >> $@
	@echo "exit" >> $@

.PHONY: pinout
pinout: $(PINOUT_SCRIPT)
	$(CUBEMX_EXE) -s $(shell realpath $(PINOUT_SCRIPT)) > $(BUILD_DIR)/cubemx.log
	python3 $(STM32X_DIR)/tools/stm32x_cubemx_gpio_export.py $(PROJECT_IOC_FILE) $(PROJECT_CSV_FILE) \
		--namespace $(PROJECT) --numeric $(PINOUT_OPTIONS) \
		--cubemx $(CUBEMX) \
		-o $(PINOUT_GPIO_PATH)

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
		-x $(SCRIPT_DIR)/bmp_flash_swd.scr \
		$(ELFFILE)

.PHONY: debug_bmp
debug_bmp: $(ELFFILE)
	$(Q)$(GDB) -ex 'target extended-remote $(BMP_PORT)' \
		-x $(SCRIPT_DIR)/bmp_gdb_swd.scr \
		$(ELFFILE)
