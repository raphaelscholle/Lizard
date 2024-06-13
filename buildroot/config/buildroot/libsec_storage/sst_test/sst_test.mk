################################################################################
#
# apps
#
################################################################################
SST_TEST_SITE_METHOD = local
SST_TEST_SITE = $(PLATFORM_PATH)/../../base/libsec_storage
SST_TEST_LICENSE = GPLv2+, GPLv3+
SST_TEST_LICENSE_FILES = Copyright COPYING

SST_TEST_DEPENDENCIES += libsst
SST_TEST_CONF_OPTS += -L$(TARGET_DIR)/usr/lib

define SST_TEST_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" -C $(@D)/demo all LDFLAGS="$(SST_TEST_CONF_OPTS)"
endef

define SST_TEST_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/demo/sst_test $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))
