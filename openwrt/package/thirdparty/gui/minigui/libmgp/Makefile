#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmgp
PKG_VERSION:=1.2.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=.
PKG_SOURCE_PROTO:=linkfile
PKG_MD5SUM:=68e6aee42895d8bddd933f35b9ad7385

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmgp
  SECTION:=gui
  SUBMENU:=Minigui
  CATEGORY:=Gui
  DEPENDS:= +libminigui-gpl
  TITLE:=libmgp
  URL:=http://www.minigui.org/
endef

define Package/libmgp/description
  mGp is one of components of MiniGUI,
  which provides support for printing on MiniGUI.
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

TARGET_LDFLAGS+=$(if $(CONFIG_USE_GLIBC),-ldl)

CONFIGURE_ARGS += \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgp \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgp*.{a,la,so*} \
		$(1)/usr/lib/
endef

define Package/libmgp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/* \
		$(1)/usr/bin/
endef

$(eval $(call BuildPackage,libmgp))
