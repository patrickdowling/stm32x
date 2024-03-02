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
$(call check_variable_list, STM32X_HAL_DIR)

SYSTEM_DEFINES += \
	CCM_RAM_SIZE=$(shell $(NUMFMT) --from=iec $(CCM_RAM_SIZE))

ifeq ($(ENABLE_CCM_STACK),TRUE)
	SYSTEM_DEFINES += ENABLE_CCM_STACK
endif

SYSTEM_DEFINES += ARM_MATH_CM4
#__FPU_PRESENT
ARCH_FLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16

STARTUP_FILE ?= "startup_$(MODEL).s"
LINKER_SCRIPT_IN = $(STM32X_MODEL_DIR)/linker/stm32f4xx_flash.ld.in

ST_CMSIS_DIR = $(STM32X_HAL_DIR)/Drivers/CMSIS
ST_HAL_DIR =$(STM32X_HAL_DIR)/Drivers/STM32F4xx_HAL_Driver
ST_CORE_DIR = $(STM32X_HAL_DIR)/Core

SYS_INCLUDES += $(ST_CMSIS_DIR)/include
SYS_INCLUDES += $(ST_CMSIS_DIR)/Device/ST/STM32F4xx/Include
SYS_INCLUDES += $(ST_HAL_DIR)/Inc
SYS_INCLUDES += $(ST_HAL_DIR)/Inc/Legacy
SYS_INCLUDES += $(ST_CORE_DIR)/Inc

SYSTEM_DEFINES += USE_FULL_LL_DRIVER
SYSTEM_DEFINES += USE_HAL_DRIVER

C_FILES += stm32f4xx_ll_gpio.c \
	   stm32f4xx_ll_rcc.c \
	   stm32f4xx_ll_spi.c \
	   stm32f4xx_ll_tim.c \
	   stm32f4xx_ll_usart.c \
	   stm32f4xx_ll_utils.c \
	   stm32f4xx_hal.c \
	   stm32f4xx_hal_cortex.c
C_FILES += system_stm32f4xx.c

AS_FILES += $(STARTUP_FILE)

VPATH += $(ST_HAL_DIR)/Src $(ST_CORE_DIR)/Src
