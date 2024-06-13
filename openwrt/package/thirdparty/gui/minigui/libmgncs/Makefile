#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmgncs
PKG_VERSION:=1.2.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=.
PKG_SOURCE_PROTO:=linkfile
PKG_MD5SUM:=3cf14f2fdacbea9cee0b526ae44be19d

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libmgncs
  SECTION:=gui
  SUBMENU:=Minigui
  CATEGORY:=Gui
  DEPENDS:= +libminigui-gpl +libmgutils +libsqlite3 +libxml2 +libstdcpp +libmgplus
  TITLE:=libmgncs
  URL:=http://www.minigui.org/
endef

define Package/libmgncs/description
  mGNCS provides a new control set for MiniGUI
  application. By using mGNCS, you can use miniStudio
  to develop MiniGUI application in WYSIWYG way.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef

#config the toolchain,which depends on arm or arm64 or riscv
#notice:arm musl,arm glibc,arm9 musl,arm9 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"arm")
MGNCS_GNU_HOST_NAME = arm-openwrt-linux
endif
#notice:aarch64 musl,aarch64 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"aarch64")
MGNCS_GNU_HOST_NAME = aarch64-openwrt-linux
endif
#notice:riscv glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"riscv")
MGNCS_GNU_HOST_NAME = riscv64-unknown-linux-gnu
endif
#end config the toolchain

TARGET_LDFLAGS+= $(if $(CONFIG_USE_GLIBC),-ldl)
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ make V=s

TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/libxml2

CONFIGURE_PREFIX:=/usr
CONFIGURE_ARGS = \
	--target=$(GNU_TARGET_NAME) \
	--host=$(MGNCS_GNU_HOST_NAME) \
	--build=$(GNU_HOST_NAME) \
	--program-prefix="" \
	--program-suffix="" \
	--prefix=$(CONFIGURE_PREFIX) \
	--exec-prefix=$(CONFIGURE_PREFIX) \
	--bindir=$(CONFIGURE_PREFIX)/bin \
	--sbindir=$(CONFIGURE_PREFIX)/sbin \
	--libexecdir=$(CONFIGURE_PREFIX)/lib \
	--sysconfdir=/etc \
	--datadir=$(CONFIGURE_PREFIX)/share/local \
	--localstatedir=/var \
	--mandir=$(CONFIGURE_PREFIX)/man \
	--infodir=$(CONFIGURE_PREFIX)/info \

define Build/Compile

endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgncs \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgncs*.{a,la,so*} \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mgncs.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libmgncs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/etc/* \
		$(1)/usr/local/etc/
endef

$(eval $(call BuildPackage,libmgncs))
