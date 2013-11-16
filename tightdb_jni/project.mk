ENABLE_INSTALL_DEBUG_LIBS = 1

# Construct fat binaries on Darwin when using Clang
ifneq ($(TIGHTDB_ENABLE_FAT_BINARIES),)
ifneq ($(call CC_CXX_AND_LD_ARE,clang),)
ifeq ($(OS),Darwin)
CFLAGS_ARCH  += -arch i386 -arch x86_64
endif
endif
endif

ifeq ($(OS),Darwin)
CFLAGS_ARCH += -mmacosx-version-min=10.7
endif

# FIXME: '-fno-elide-constructors' currently causes TightDB to fail
#CFLAGS_DEBUG   += -fno-elide-constructors
CFLAGS_PTHREAD += -pthread
CFLAGS_GENERAL += -Wextra -ansi -pedantic -Wno-long-long

# Load dynamic configuration
ifeq ($(NO_CONFIG_MK),)
CONFIG_MK = $(GENERIC_MK_DIR)/config.mk
DEP_MAKEFILES += $(CONFIG_MK)
include $(CONFIG_MK)
endif
