#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libminigui-gpl
PKG_VERSION:=3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=.
PKG_SOURCE_PROTO:=linkfile
PKG_MD5SUM:=9e8f36965c0a113050cbe3d31236878a

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libminigui-gpl
  SECTION:=gui
  SUBMENU:=Minigui
  CATEGORY:=Gui
  DEPENDS:= +zlib +libpthread +libpng +libjpeg +libfreetype +libuapi +tslib +minigui-res-be \
            +MINIGUI_G2D_SUPPORT:kmod-sunxi-g2d
  TITLE:=libminigui-gpl
  URL:=http://www.minigui.org/
  MENU:=1
endef

define Package/libminigui-gpl/description
  The ligminigui library of GUI Core
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -r ./src/sunxifb $(PKG_BUILD_DIR)/src/newgal/sunxifb
	$(CP) -r ./src/sunxifbion $(PKG_BUILD_DIR)/src/newgal/sunxifbion
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef

define Package/libminigui-gpl/config
  source "$(SOURCE)/Config.in"
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

TARGET_LDFLAGS+= -L$(STAGING_DIR)/usr/lib/
TARGET_CFLAGS+=-I$(PKG_BUILD_DIR)/include
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/
TARGET_LDFLAGS+= -lm
LDFLAGS+=-lm
ifeq ($(CONFIG_MINIGUI_TTF2_SUPPORT),y)
TARGET_LDFLAGS+= -lfreetype
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/freetype2
LDFLAGS+=-lfreetype
endif
ifeq ($(CONFIG_MINIGUI_TSLIB_SUPPORT),y)
TARGET_LDFLAGS+= -lts
LDFLAGS+=-lts
endif
ifeq ($(CONFIG_MINIGUI_G2D_SUPPORT),y)
TARGET_LDFLAGS+= -luapi
LDFLAGS+=-luapi
endif
CONFIGURE_ARGS += \
	--disable-videopcxvfb \
	--disable-splash \
	--disable-screensaver \
	--enable-cursor=yes \
	--enable-videoshadow=yes \
	--with-targetname=fbcon \
	--enable-detaildebug=yes \
	--enable-ctrltreeview=yes \
	--enable-ctrliconview=yes \
	--enable-ctrlmonthcal=yes \
	--enable-ctrlbidisledit=yes \
	--enable-ctrlcoolbar=yes

LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lm -lfreetype -lts
ifeq ($(CONFIG_LINUX_5_4),y)
TARGET_CFLAGS+=-DCONF_G2D_VERSION_NEW
endif
ifeq ($(CONFIG_MINIGUI_THS),y)
CONFIGURE_ARGS += --with-runmode=ths
endif
ifeq ($(CONFIG_MINIGUI_PROC),y)
CONFIGURE_ARGS += --with-runmode=procs
endif
ifeq ($(CONFIG_MINIGUI_TTF2_SUPPORT),y)
CONFIGURE_ARGS += --with-ttfsupport=ft2
CONFIGURE_ARGS += --with-ft2-includes=$(STAGING_DIR)/usr/include/freetype2
endif
ifeq ($(CONFIG_MINIGUI_TSLIB_SUPPORT),y)
CONFIGURE_ARGS += --enable-tslibial
endif
ifeq ($(CONFIG_MINIGUI_G2D_SUPPORT),y)
CONFIGURE_ARGS += --enable-g2dsupport=yes
endif
ifeq ($(CONFIG_MINIGUI_G2D_ROTATE_SUPPORT),y)
CONFIGURE_ARGS += --enable-g2drotatesupport=yes
endif
ifeq ($(CONFIG_MINIGUI_GPU_SUPPORT),y)
CONFIGURE_ARGS += --enable-gpusupport=yes
endif
ifeq ($(CONFIG_MINIGUI_GPU_ROTATE_SUPPORT),y)
CONFIGURE_ARGS += --enable-gpurotatesupport=yes
endif
ifeq ($(CONFIG_MINIGUI_SUNXIFB_SUPPORT),y)
CONFIGURE_ARGS += --enable-videosunxifb=yes
endif
ifeq ($(CONFIG_MINIGUI_SUNXIFBION_SUPPORT),y)
CONFIGURE_ARGS += --enable-videosunxifbion=yes
endif
ifeq ($(CONFIG_MINIGUI_TEXT_ROTATE_SUPPORT),y)
CONFIGURE_ARGS += --enable-textrotatesupport=yes
endif
ifeq ($(CONFIG_MINIGUI_CCW_SUPPORT),y)
CONFIGURE_ARGS += --enable-coortrans_cw
endif
ifeq ($(CONFIG_MINIGUI_BUILD_LIB_MIN),y)
CONFIGURE_ARGS += \
	--enable-flatlf=no \
	--enable-skinlf=no \
	--enable-cursor=no \
	--enable-clipboard=no \
	--enable-customial=no \
	--enable-dlcustomial=no \
	--enable-dummyial=yes \
	--enable-consoleial=yes \
	--enable-consoleps2=no \
	--enable-consoleimps2=yes \
	--enable-consolems=no \
	--enable-consolems3=no \
	--enable-textmode=no \
	--enable-rbfsupport=yes \
	--enable-rbfvgaoem=no \
	--enable-rbfterminal=no \
	--enable-rbffixedsys=yes \
	--enable-vbfsupport=no \
	--enable-fontsserif=no \
	--enable-fontcourier=no \
	--enable-fontsystem=no \
	--enable-upfsupport=no \
	--enable-fonttimes=no \
	--enable-bmpfsupport=no \
	--enable-latin2support=no \
	--enable-latin3support=no \
	--enable-latin4support=no \
	--enable-cyrillicsupport=no \
	--enable-arabicsupport=no \
	--enable-greeksupport=no \
	--enable-hebrewsupport=no \
	--enable-latin5support=no \
	--enable-latin6support=no \
	--enable-thaisupport=no \
	--enable-latin7support=no \
	--enable-latin8support=no \
	--enable-latin9support=no \
	--enable-latin10support=no \
	--enable-gbsupport=no \
	--enable-gbksupport=no \
	--enable-big5support=no \
	--enable-euckrsupport=no \
	--enable-eucjpsupport=no \
	--enable-shiftjissupport=no \
	--enable-unicodesupport=yes \
	--enable-savebitmap=yes \
	--enable-gifsupport=yes \
	--enable-jpgsupport=yes \
	--enable-pngsupport=yes \
	--enable-menu=yes \
	--enable-mousecalibrate=no \
	--enable-aboutdlg=no \
	--enable-savescreen=no \
	--enable-ctrlstatic=yes \
	--enable-ctrlbutton=yes \
	--enable-ctrlsledit=no \
	--enable-ctrlbidisledit=no \
	--enable-ctrllistbox=no \
	--enable-ctrlpgbar=yes \
	--enable-ctrlnewtoolbar=no \
	--enable-ctrlmenubtn=no \
	--enable-ctrltrackbar=no \
	--enable-ctrlcombobox=no \
	--enable-ctrlpropsheet=no \
	--enable-ctrlscrollview=yes \
	--enable-ctrlmonthcal=no \
	--enable-ctrltreeview=no \
	--enable-ctrlspinbox=no \
	--enable-ctrlcoolbar=no \
	--enable-ctrllistview=no \
	--enable-ctrliconview=no \
	--enable-ctrlgridview=no \
	--enable-ctrlanimation=no \
	--enable-ctrlscrollbar=yes \
	--enable-newtextedit=no \
	--enable-videodummy=no \
	--enable-videofbcon=yes \
	--enable-videoqvfb=no \
	--enable-videopcxvfb=no
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DIR) $(1)/usr/share/minigui/cmake/Modules
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/minigui \
		$(1)/usr/include/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libminigui_*.{a,la,so*} \
		$(1)/usr/lib/

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/minigui.pc \
		$(1)/usr/lib/pkgconfig/

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/minigui/cmake/Modules/*.cmake \
		$(1)/usr/share/minigui/cmake/Modules/
endef

define Package/libminigui-gpl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(INSTALL_DIR) $(1)/etc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
#
# Because of OPKG_INCOMPATIBILITY, we install the following files 
# (MiniGUI.cfg, sunxi-keyboard.kl) once in the application.
#
#	$(CP) \
#		./MiniGUI.cfg \
#		$(1)/usr/local/etc/
#	$(CP) \
#		./sunxi-keyboard.kl \
#		$(1)/etc/
#
endef

$(eval $(call BuildPackage,libminigui-gpl))
