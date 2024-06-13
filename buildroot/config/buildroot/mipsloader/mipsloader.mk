################################################################################
#
# libdispaly package
#
################################################################################
MIPSLOADER_SITE_METHOD = local
MIPSLOADER_SITE = $(PLATFORM_PATH)/../../framework/display/mipsloader

MIPSLOADER_LICENSE = GPLv2+, GPLv3+
MIPSLOADER_LICENSE_FILES = Copyright COPYING
MIPSLOADER_INSTALL_STAGING = YES

MIPSLOADER_DEPENDENCIES += libsst

MIPSLOADER_CFLAGS += -I$(LICHEE_BR_OUT)/build/libsst/src/include
MIPSLOADER_CPPFLAGS += -I$(LICHEE_BR_OUT)/build/libsst/src/include
MIPSLOADER_LDFLAGS += -L$(TARGET_DIR)/usr/lib

define MIPSLOADER_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" \
	CPPFLAGS="$(MIPSLOADER_CPPFLAGS)" LDFLAGS="$(MIPSLOADER_LDFLAGS)" \
	SRCDIR=$(@D) -C $(@D) -f $(@D)/Makefile_kunos
endef

define MIPSLOADER_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/loadmips $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))
