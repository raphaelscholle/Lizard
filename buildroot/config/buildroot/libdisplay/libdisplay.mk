################################################################################
#
# libdispaly package
#
################################################################################
LIBDISPLAY_SITE_METHOD = local
LIBDISPLAY_SITE = $(PLATFORM_PATH)/../../framework/display/libdisp

LIBDISPLAY_LICENSE = GPLv2+, GPLv3+
LIBDISPLAY_LICENSE_FILES = Copyright COPYING
LIBDISPLAY_INSTALL_STAGING = YES

#LIBSYS_INFO_DEPENDENCIES = 
LIBDIPSLAY_CFLAGS = $(TARGET_CFLAGS)
LIBDISPLAY_CFLAGS += -O2 -fPIC -g -Wall -DCOMPLETE_TIMEOUT
LIBDISPLAY_CFLAGS += -I$(LICHEE_BR_OUT)/build/libdisplay/include
LIBDISPLAY_LDFLAGS = $(TARGET_LDFLAGS)
LIBDISPLAY_LDFLAGS += -L$(TARGET_DIR)/usr/lib/ -lpthread -shared

define LIBDISPLAY_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" SRCDIR=$(@D) -C $(@D)/ -f $(@D)/Makefile_kunos
endef

define LIBDISPLAY_INSTALL_STAGING_CMDS
	$(INSTALL) -D -m 0644 $(@D)/libdisp.so $(STAGING_DIR)/usr/lib/
endef

define LIBDISPLAY_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0644 $(@D)/libdisp.so $(TARGET_DIR)/usr/lib/
endef

$(eval $(generic-package))
