#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=multimedia-test
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=gui
  SUBMENU:=Minigui
  CATEGORY:=Gui
  DEPENDS:= +libminigui-gpl +alsa-utils +tslib +libcedarx +libuapi
  TITLE:=multimedia-test
  MAINTAINER:=arl<anruliu@allwinnertech.com>
endef

define Package/$(PKG_NAME)/description
  The minigui application for multimedia-test.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	mkdir -p $(PKG_BUILD_DIR)/src
	mkdir -p $(PKG_BUILD_DIR)/res
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./res $(PKG_BUILD_DIR)/

	$(CP) ./machinfo/common/* $(PKG_BUILD_DIR)/res/
ifeq ("$(shell if [ -d ./machinfo/$(LICHEE_IC) ]; then echo "exist"; else echo "noexist"; fi)", "exist")
	$(info *****************LICHEE_IC = $(LICHEE_IC)**************)
	$(CP) ./machinfo/$(LICHEE_IC)/* $(PKG_BUILD_DIR)/res/
	$(CP) ./machinfo/$(LICHEE_IC)/SetAudioPass/multimedia-test.sh $(PKG_BUILD_DIR)/res/SetAudioPass
endif
endef

TARGET_LDFLAGS+=-lfreetype

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/res
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/res/
	$(CP) \
		$(PKG_BUILD_DIR)/src/multimedia-test \
		$(1)/usr/bin/
	$(CP) \
		$(PKG_BUILD_DIR)/res/* \
		$(1)/usr/res/
	$(CP) \
		$(PKG_BUILD_DIR)/res/wm_res/* \
		$(1)/etc/res/
	$(CP) \
		$(PKG_BUILD_DIR)/res/minigui/MiniGUI.cfg \
		$(1)/usr/local/etc/
	$(CP) \
		$(PKG_BUILD_DIR)/src/pointercal \
		$(1)/etc/
	$(CP) \
		$(PKG_BUILD_DIR)/res/minigui/sunxi-keyboard.kl \
		$(1)/etc/
	$(CP) ./start_multimedia.sh $(1)/usr/bin/
#	$(INSTALL_BIN) ./multimedia_autorun.init $(1)/etc/init.d/multimedia_autorun
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
