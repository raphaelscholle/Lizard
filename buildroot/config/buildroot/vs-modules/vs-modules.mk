################################################################################
#
# apps
#
################################################################################
VS_MODULES_SITE_METHOD = local
VS_MODULES_SITE = $(PLATFORM_PATH)/../../vendor/vs
VS_MODULES_LICENSE = GPLv2+, GPLv3+
VS_MODULES_LICENSE_FILES = Copyright COPYING

VS_MODULES_DEPENDENCIES += \
	libsst

define VS_MODULES_BUILD_CMDS
	cp -rf ${LICHEE_TOP_DIR}/platform/config/buildroot/vs-modules/scripts/* $(@D)/
	source $(@D)/build.sh
endef

define VS_MODULES_INSTALL_TARGET_CMDS
	mkdir -p ${LICHEE_TOP_DIR}/platform/target/${LICHEE_IC}/${LICHEE_BOARD}/busybox-init-base-files/lib/vs/modules
	mkdir -p ${LICHEE_TOP_DIR}/platform/target/${LICHEE_IC}/${LICHEE_BOARD}/busybox-init-base-files/opt/test/release
	mkdir -p ${LICHEE_TOP_DIR}/platform/target/${LICHEE_IC}/${LICHEE_BOARD}/busybox-init-base-files/opt/vs_rootfs
	tar zxvf $(@D)/VS_rootfs/WorkDir.tgz -C ${LICHEE_TOP_DIR}/platform/target/${LICHEE_IC}/${LICHEE_BOARD}/busybox-init-base-files/opt/vs_rootfs
	cp -rf $(@D)/Install/Core/Kernel_Driver/*.ko ${LICHEE_TOP_DIR}/platform/target/${LICHEE_IC}/${LICHEE_BOARD}/busybox-init-base-files/lib/vs/modules
	cp -rf $(@D)/Dispmips_vs_bin/*.TSE ${LICHEE_TOP_DIR}/platform/target/${LICHEE_IC}/${LICHEE_BOARD}/busybox-init-base-files/opt/test/release
	cp -rf $(@D)/Dispmips_vs_bin/*.bin ${LICHEE_TOP_DIR}/platform/target/${LICHEE_IC}/${LICHEE_BOARD}/busybox-init-base-files/opt/test/release
	cp -rf $(@D)/Dispmips_vs_bin/*.xml ${LICHEE_TOP_DIR}/platform/target/${LICHEE_IC}/${LICHEE_BOARD}/busybox-init-base-files/opt/test/release
	cp -rf $(@D)/Install/Core/Lib/*.so $(TARGET_DIR)/usr/lib/
	cp -rf $(@D)/Install/Core/Bin/* $(TARGET_DIR)/usr/bin/
endef
$(eval $(generic-package))
