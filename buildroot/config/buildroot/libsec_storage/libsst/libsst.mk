################################################################################
#
# apps
#
################################################################################
LIBSST_SITE_METHOD = local
LIBSST_SITE = $(PLATFORM_PATH)/../../base/libsec_storage
LIBSST_LICENSE = GPLv2+, GPLv3+
LIBSST_LICENSE_FILES = Copyright COPYING
LIBSST_INSTALL_STAGING = YES

define LIBSST_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" -C $(@D)/src/ all
endef

define LIBSST_INSTALL_STAGING_CMDS
	mkdir -p $(STAGING_DIR)/usr/include/secure_storage/
	cp $(@D)/src/include/*.h $(STAGING_DIR)/usr/include/secure_storage/
endef

define LIBSST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/src/libsst.so $(TARGET_DIR)/usr/lib/
endef

$(eval $(generic-package))
