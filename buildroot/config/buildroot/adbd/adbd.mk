################################################################################
#
# adbd package
#
################################################################################
ADBD_SITE_METHOD = local
ADBD_SITE = ../../platform/allwinner/usb/adbd/src
AUTH_SITE = ../../platform/allwinner/usb/adbd/auth

ADBD_LICENSE = GPLv2+, GPLv3+
ADBD_LICENSE_FILES = Copyright COPYING

ADBD_CFLAGS = $(TARGET_CFLAGS)
ADBD_CFLAGS += -O2 -fPIC -Wall -I$(@D) -I$(@D)/libs/libcutils -I$(@D)/libs/libmincrypt
ADBD_CFLAGS += -I$(STAGING_DIR)/usr/include -O2 -g -DADB_HOST=0 -Wall -Wno-unused-parameter 
ADBD_CFLAGS += -DALLOW_ADBD_ROOT=1 -DHAVE_FORKEXEC -D_XOPEN_SOURCE -D_GNU_SOURCE

ADBD_LDFLAGS = $(TARGET_LDFLAGS)
ADBD_LDFLAGS += -L$(TARGET_DIR)/usr/lib/ -lpthread libmincrypt.a libcutils.a

define ADBD_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" CFLAGS="$(ADBD_CFLAGS)" \
		LDFLAGS="$(TARGET_DIR)" -C $(@D)/libs/libmincrypt

	$(MAKE) CC="$(TARGET_CC)" CFLAGS="$(ADBD_CFLAGS)" \
		LDFLAGS="$(TARGET_DIR)" -C $(@D)/libs/libcutils

	cp -af $(@D)/*/*/*.a $(@D)/

	$(MAKE) CC="$(TARGET_CC)" CFLAGS="$(ADBD_CFLAGS)" \
		LDFLAGS="$(ADBD_LDFLAGS)" -C $(@D)
endef

define ADBD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/adbd $(TARGET_DIR)/bin
endef

$(eval $(generic-package))
