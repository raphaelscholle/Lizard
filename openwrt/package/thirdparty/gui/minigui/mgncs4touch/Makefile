#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mgncs4touch
PKG_VERSION:=1.2.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=.
PKG_SOURCE_PROTO:=linkfile
PKG_MD5SUM:=fbdbfad2dc508db6cae2bc49e003211e

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/${PKG_NAME}
  SECTION:=gui
  SUBMENU:=Minigui
  CATEGORY:=Gui
  DEPENDS:= +libminigui-gpl +libmgplus +libmgeff +libmgncs +libchipmunk
  TITLE:=mgncs4touch
  URL:=http://www.minigui.org/
endef

define Package/$(PKG_NAME)/description
endef

#config the toolchain,which depends on arm or arm64 or riscv
#notice:arm musl,arm glibc,arm9 musl,arm9 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"arm")
CONFIGURE_ARGS += --host=arm-openwrt-linux
endif
#notice:aarch64 musl,aarch64 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"aarch64")
CONFIGURE_ARGS += --host=aarch64-openwrt-linux
endif
#notice:riscv glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"riscv")
CONFIGURE_ARGS += --host=riscv64-unknown-linux-gnu
endif
#end config the toolchain

TARGET_LDFLAGS+= $(if $(CONFIG_USE_GLIBC),-ldl)

ifeq ($(CONFIG_MINIGUI_TSLIB_SUPPORT),y)
TARGET_LDFLAGS+= -lts
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lts

endif

ifeq ($(CONFIG_MINIGUI_G2D_SUPPORT),y)
TARGET_LDFLAGS+= -luapi
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -luapi
endif
define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgncs4touch \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgncs4touch*.{a,la,so*} \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mgncs4touch.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DIR) $(1)/usr/include/mgncs4touch
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgncs4touch*.{a,la,so*} \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgncs4touch/* \
		$(1)/usr/include/mgncs4touch
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mgncs4touch.pc \
		$(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_BUILD_DIR)/etc/mgncs4touch.cfg \
		$(1)/usr/local/etc
endef

$(eval $(call BuildPackage,${PKG_NAME}))
