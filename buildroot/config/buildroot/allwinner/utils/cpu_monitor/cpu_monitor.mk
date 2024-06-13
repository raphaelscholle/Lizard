################################################################################
#
# apps
#
################################################################################
CPU_MONITOR_SITE_METHOD = local
CPU_MONITOR_SITE = $(PLATFORM_PATH)/../../tools/debug/cpu_monitor
CPU_MONITOR_LICENSE = GPLv2+, GPLv3+
CPU_MONITOR_LICENSE_FILES = Copyright COPYING

define CPU_MONITOR_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/$(BR2_TOOLCHAIN_EXTERNAL_PREFIX)/* $(TARGET_DIR)/usr/bin
endef

$(eval $(generic-package))
