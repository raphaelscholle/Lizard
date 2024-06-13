################################################################################
#
# apps
#
################################################################################
WIFI_FIRMWARE_SITE_METHOD = local
WIFI_FIRMWARE_SITE = $(PLATFORM_PATH)/../../core/net/wifi-firmware/$(wifi_name)
WIFI_FIRMWARE_LICENSE = GPLv2+, GPLv3+
WIFI_FIRMWARE_LICENSE_FILES = Copyright COPYING

wifi_name=$(shell cat $(LICHEE_BR_OUT)/.config | grep "WIFI_NAME" | awk -F "\"" '{print $$2}')

define WIFI_FIRMWARE_INSTALL_TARGET_CMDS
	source $(@D)/firmware_copy.sh
endef
$(eval $(generic-package))
