################################################################################
#
# apps
#
################################################################################
MTOP_SITE_METHOD = local
MTOP_SITE = $(PLATFORM_PATH)/../../tools/debug/mtop
MTOP_LICENSE = GPLv2+, GPLv3+
MTOP_LICENSE_FILES = Copyright COPYING

define MTOP_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" -C $(@D) all
endef

define MTOP_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/mtop $(TARGET_DIR)/usr/bin
	$(INSTALL) -D -m 0644 $(@D)/mtop.cfg $(TARGET_DIR)/etc/
endef

$(eval $(generic-package))
