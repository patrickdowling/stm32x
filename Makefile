include mk/utils.mk

# clang-format
CLANG_FORMAT_OPTS += -i --style=file
ifdef VERBOSE
CLANG_FORMAT_OPTS += --verbose
endif

CLANG_FORMAT_DIRS = include/ src/
CLANG_FORMAT_SHALLOW_DIRS = test/

.PHONY: help
help:
	@echo "Available targets"
	@echo "make format (apply clang-format)"

.PHONY: format
format:
	$(Q)clang-format $(CLANG_FORMAT_OPTS) \
		$(foreach dir,$(CLANG_FORMAT_DIRS),$(shell find $(dir) -name \*.cc -o -name \*.h))
	$(Q)clang-format $(CLANG_FORMAT_OPTS) \
		$(foreach dir,$(CLANG_FORMAT_SHALLOW_DIRS),$(shell find $(dir) -depth 1 -name \*.cc -o -depth 1 -name \*.h))
