###
## Quick & dirty makefile for test "suite"
#

TARGET=stm32x_test

# DIRECTORIES & CONFIG
BUILD_DIR = ./build/$(TARGET)/
STM32X_DIR = ../
SRC_DIRS = .

RM    = rm -f
RMDIR = rmdir
MKDIR = mkdir -p
CXX   = g++
LD    = g++
AR    = ar -r

DEFINES += TESTING
INCLUDES = $(GTEST_DIR)include $(STM32X_DIR)
INCLUDES += 

CPPFLAGS += -std=c++11
CPPFLAGS += -Wall -Werror
CPPFLAGS += $(addprefix -D, $(DEFINES))
CPPFLAGS += $(addprefix -I, $(INCLUDES)) $(addprefix -I, $(SRC_DIRS))
CPPFLAGS += -MMD -MP

# GTEST
GTEST_DIR = ./gtest/googletest/
LIBGTEST = $(BUILD_DIR)libgtest.a

# SOURCE FILES
VPATH = . $(SRC_DIRS)
CC_FILES = $(notdir $(wildcard $(patsubst %,%/*.cc,$(SRC_DIRS))))
OBJ_FILES = $(CC_FILES:.cc=.o)
OBJS      = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES))
DEPS      = $(OBJS:.o=.d)

EXE = $(BUILD_DIR)$(TARGET)

# COMPILER RULES
$(BUILD_DIR)%.o: %.cc
	$(CXX) -c $(CCFLAGS) $(CPPFLAGS) $< -o $@

# TARGETS
.PHONY: all
all: runtests

.PHONY: runtests
runtests: $(EXE)
	@$(EXE) $(GTEST_FILTER)

$(EXE): $(BUILD_DIR) $(LIBGTEST) $(OBJS)
	@echo "Linking $(EXE)..."
	@$(LD) $(LDFLAGS) -o $(EXE) $(OBJS) $(LIBGTEST)

$(BUILD_DIR):
	@$(MKDIR) $(BUILD_DIR)

$(LIBGTEST): $(BUILD_DIR)
	@$(CXX) -isystem $(GTEST_DIR)include -I$(GTEST_DIR) -pthread -c $(GTEST_DIR)src/gtest-all.cc -o $(BUILD_DIR)gtest-all.o
	@$(AR) $(LIBGTEST) $(BUILD_DIR)/gtest-all.o

.PHONY: clean
clean:
	@$(RM) $(LIBGTEST) $(OBJS) $(EXE) $(DEPS)

-include $(DEPS)
