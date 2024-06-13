#
# Copyright (C) 2015-2021 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=flutter-sunxi
PKG_VERSION:=1.0.7
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=.
PKG_SOURCE_PROTO:=linkfile
PKG_HASH:=c8e305d0ad1bb3ada58adb560aac4754d7605d2f01335642e7c234962048ee74

PKG_MAINTAINER:=anruliu <anruliu@allwinnertech.com>
PKG_LICENSE:=BSD 3-Clause "New" or "Revised" License
PKG_LICENSE_FILES:=LICENSE

ifeq ($(ARCH), arm)
FLUTTER_ARCH:=arm32
else
FLUTTER_ARCH:=arm64
endif

PKG_CONFIG_DEPENDS := \
	CONFIG_FLUTTER_USE_FBDEV \
	CONFIG_FLUTTER_USE_EGLFS \
	CONFIG_FLUTTER_USE_CILENT \
	CONFIG_FLUTTER_USE_ELINUX_SO \
	CONFIG_FLUTTER_DEMO_COMPLEX_LAYOUT \
	CONFIG_FLUTTER_DEMO_GALLERY \
	CONFIG_FLUTTER_DEMO_VIDEO_PLAYER

include $(INCLUDE_DIR)/package.mk

define Package/flutter-sunxi
  TITLE:=flutter-sunxi
  SECTION:=libs
  SUBMENU:=Flutter
  CATEGORY:=Gui
  URL:=https://flutter.dev/
  DEPENDS:=@USE_GLIBC +FLUTTER_USE_EGLFS:libgpu +FLUTTER_USE_CILENT||FLUTTER_USE_ELINUX_SO:libxkbcommon
  MENU:=1
endef

define Package/flutter-sunxi/description
	Flutter transforms the app development process. Build, test, and deploy beautiful mobile, web, desktop, and embedded apps from a single codebase.
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	$(Build/Prepare/Default)
ifeq ($(CONFIG_FLUTTER_DEMO_COMPLEX_LAYOUT),y)
	$(TAR) -zxvf $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/complex_layout.tar.gz -C $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)
endif
ifeq ($(CONFIG_FLUTTER_DEMO_GALLERY),y)
	$(TAR) -zxvf $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/gallery.tar.gz -C $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)
endif
ifeq ($(CONFIG_FLUTTER_DEMO_VIDEO_PLAYER),y)
	$(TAR) -zxvf $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/video_player.tar.gz -C $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)
endif
endef

define Build/Compile
endef

define Package/flutter-sunxi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/libflutter_engine.so $(1)/usr/lib
ifeq ($(CONFIG_FLUTTER_USE_FBDEV),y)
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/flutter_fbdev $(1)/usr/bin
endif
ifeq ($(CONFIG_FLUTTER_USE_EGLFS),y)
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/flutter_eglfs $(1)/usr/bin
endif
ifeq ($(CONFIG_FLUTTER_USE_CILENT),y)
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/flutter-client $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/libvideo_player_plugin.so $(1)/usr/lib
endif
ifeq ($(CONFIG_FLUTTER_USE_ELINUX_SO),y)
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/libflutter_elinux_eglfs.so $(1)/usr/lib
endif
ifeq ($(CONFIG_FLUTTER_DEMO_COMPLEX_LAYOUT),y)
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/bundle_complex_layout $(1)/usr/bin
endif
ifeq ($(CONFIG_FLUTTER_DEMO_GALLERY),y)
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/bundle_gallery $(1)/usr/bin
endif
ifeq ($(CONFIG_FLUTTER_DEMO_VIDEO_PLAYER),y)
	$(CP) $(PKG_BUILD_DIR)/$(FLUTTER_ARCH)/bundle $(1)/usr/bin
endif
endef

$(eval $(call BuildPackage,flutter-sunxi))
