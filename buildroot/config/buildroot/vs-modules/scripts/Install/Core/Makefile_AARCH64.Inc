#Use the default one of toolchain.
#TV303 has the same toolchain with UNION1

# for kernel modules
CROSS_ROOT := $(LICHEE_TOOLCHAIN_PATH)
CROSS_COMPILE := $(CROSS_ROOT)/bin/aarch64-linux-gnu-

ifneq (,$(CCACHE))
CCACHE += 
endif

USER_SPACE_CROSS_ROOT := ${LICHEE_BR_OUT}/host
USER_SPACE_CROSS_COMPILE := $(USER_SPACE_CROSS_ROOT)/bin/arm-linux-gnueabihf-

LD      :=$(CCACHE)$(USER_SPACE_CROSS_COMPILE)gcc
CC      :=$(CCACHE)$(USER_SPACE_CROSS_COMPILE)gcc
CXX     :=$(CCACHE)$(USER_SPACE_CROSS_COMPILE)g++
CPP     :=$(CC) -E
AR      :=$(USER_SPACE_CROSS_COMPILE)ar
NM      :=$(USER_SPACE_CROSS_COMPILE)nm
STRIP   :=$(USER_SPACE_CROSS_COMPILE)strip
OBJCOPY :=$(USER_SPACE_CROSS_COMPILE)objcopy
OBJDUMP :=$(USER_SPACE_CROSS_COMPILE)objdump
RANLIB	:=$(USER_SPACE_CROSS_COMPILE)ranlib

ifneq ( , $(USER_SPACE_CROSS_ROOT))
TOOLCHAIN_ROOT := --sysroot=$(USER_SPACE_CROSS_ROOT)/arm-buildroot-linux-gnueabihf/sysroot
endif

CPUARCH_FLAGS := -mcpu=cortex-a53 -marm -mlittle-endian -mfpu=neon -mfloat-abi=hard
CPUARCH_FLAGS_64BIT := -mcpu=cortex-a53 -mlittle-endian
DEFS := -fPIC -D_REENTRANT -DPURE_LITTLE_ENDIAN
OPT := -O2 -fno-strict-aliasing $(CPUARCH_FLAGS)
OPT_64BIT := -O2 -fno-strict-aliasing $(CPUARCH_FLAGS_64BIT)
BASE_FLAGS := -Wall -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-unused-local-typedefs -Wno-sizeof-pointer-memaccess -Wno-maybe-uninitialized
CFLAGS := $(TOOLCHAIN_ROOT) $(OPT) $(DEBUG) $(DEFS) $(BASE_FLAGS) -Wstrict-prototypes -fgnu89-inline
CPPFLAGS := $(TOOLCHAIN_ROOT) $(OPT) $(DEBUG) $(DEFS) $(BASE_FLAGS)
KERNEL_CFLAGS := $(OPT_64BIT) -DCONFIG_ENDIAN_LITTLE $(BASE_FLAGS)
ARFLAGS := ru
LDFLAGS := $(TOOLCHAIN_ROOT)
HOST := arm-linux
64KO_TOOLCHAIN_PATH:=
CONFIG_64BIT_KO := 1
