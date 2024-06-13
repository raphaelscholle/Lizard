################################################################################
#
# apps
#
################################################################################
CPU_MONITOR_SITE_METHOD = local
CPU_MONITOR_SITE = $(PLATFORM_PATH)/../../../platform/allwinner/utils/cpu_monitor/src
CPU_MONITOR_LICENSE = GPLv2+, GPLv3+
CPU_MONITOR_LICENSE_FILES = Copyright COPYING

define CPU_MONITOR_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" -C $(@D) all
endef

define CPU_MONITOR_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/cpu_monitor $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
