#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmgplus
PKG_VERSION:=1.4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=.
PKG_SOURCE_PROTO:=linkfile
PKG_MD5SUM:=550b99b4472b28a82f726da411e8f214

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=gui
  SUBMENU:=Minigui
  CATEGORY:=Gui
  DEPENDS:= +libminigui-gpl +libstdcpp
  TITLE:=libmgplus
  URL:=http://www.minigui.org/
endef

define Package/$(PKG_NAME)/description
  mGPlus is a component of MiniGUI, which provides 
  support for advanced graphics function. 
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
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

TARGET_LDFLAGS+= -lm
LDFLAGS+=-lm

CONFIGURE_ARGS += --enable-ft2support

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgplus \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mgplus.pc \
		$(1)/usr/lib/pkgconfig/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgplus*.{a,la,so*} \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/share/minigui/cmake/Modules
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/minigui/cmake/Modules/*.cmake \
		$(1)/usr/share/minigui/cmake/Modules/

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
