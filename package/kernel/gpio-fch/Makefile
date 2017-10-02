#
# Copyright (C) 2017 Florian Eckert <fe@dev.tdt.de>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=gpio-fch
PKG_RELEASE:=1

PKG_MAINTAINER:=Florian Eckert <fe@dev.tdt.de>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define KernelPackage/gpio-fch
  SUBMENU:=Other modules
  TITLE:= GPIO for AMD FCH support
  DEPENDS:= @PCI_SUPPORT @GPIO_SUPPORT @TARGET_x86
  FILES:=$(PKG_BUILD_DIR)/gpio-fch.ko
  AUTOLOAD:=$(call AutoLoad,41,gpio-fch,1)
  KCONFIG:=
endef

define KernelPackage/gpio-fch/description
  Support for GPIO functionality of AMD FCH
endef

EXTRA_KCONFIG:= \
	CONFIG_GPIO_FCH=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	 $(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,gpio-fch))
