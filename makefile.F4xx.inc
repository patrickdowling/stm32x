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
# F4xx specifics

$(call check_variable_list, CCM_RAM_SIZE HSE_VALUE)

SYSTEM_DEFINES += \
	CCM_RAM_SIZE=$(shell $(NUMFMT) --from=iec $(CCM_RAM_SIZE))

ifeq ($(ENABLE_CCM_STACK),TRUE)
	SYSTEM_DEFINES += ENABLE_CCM_STACK
endif

STARTUP_FILE ?= startup_stm32f40xx.s
LINKER_SCRIPT_IN = $(STM32X_DIR)linker/stm32f4xx_flash.ld.in
SYSTEM_DEFINES += GCC_ARMCM4 STM32X_F4XX
ARCH_FLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16

STM32_CMSIS_DIR=$(ST_DIR)CMSIS/
STM32_STDPERIPH_DIR=$(ST_DIR)STM32F4xx_StdPeriph_Driver/

INCLUDES += $(STM32_CMSIS_DIR)
INCLUDES += $(STM32_CMSIS_DIR)Include/
INCLUDES += $(STM32_CMSIS_DIR)STM32F4xx/Include/
INCLUDES += $(STM32_STDPERIPH_DIR)inc/

PROJECT_SRC_DIRS += $(STM32_STDPERIPH_DIR)src
PROJECT_SRC_DIRS += $(STM32X_DIR)stm32f4xx

C_FILES += system_stm32f4xx.c
AS_FILES += $(STARTUP_FILE)
VPATH += $(STM32_CMSIS_DIR)STM32F4xx/Source/
