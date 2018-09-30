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
# F0 specifics

STARTUP_FILE = startup_stm32f0xx.s
LINKER_SCRIPT_IN = $(STM32X_DIR)linker/stm32f0xx_flash.ld.in
SYSTEM_DEFINES += GCC_ARMCM0 STM32X_F0XX ARM_MATH_CM0
ARCH_FLAGS += -mcpu=cortex-m0

STM32_CMSIS_DIR=$(ST_DIR)CMSIS/
STM32_STDPERIPH_DIR=$(ST_DIR)STM32F0xx_StdPeriph_Driver/

INCLUDES += $(STM32_CMSIS_DIR)
INCLUDES += $(STM32_CMSIS_DIR)Include/
INCLUDES += $(STM32_CMSIS_DIR)STM32F0xx/Include/
INCLUDES += $(STM32_STDPERIPH_DIR)inc/

PROJECT_SRC_DIRS += $(STM32_STDPERIPH_DIR)src
PROJECT_SRC_DIRS += $(STM32X_DIR)stm32f0

C_FILES += system_stm32f0xx.c
AS_FILES += $(STARTUP_FILE)
VPATH += $(STM32_CMSIS_DIR)STM32F0xx/Source/
