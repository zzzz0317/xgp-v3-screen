include $(TOPDIR)/rules.mk

PKG_NAME:=xgp-v3-screen
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_LICENSE:=GPL-3.0-only

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/xgp-v3-screen
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=NLnet XiGuaPi V3 TFT Screen
	DEPENDS:=+python3 +libpthread +libstdcpp +kmod-fb-tft-gc9307
	URL:=https://github.com/zzzz0317/xgp-v3-screen
endef

define Package/xgp-v3-screen/description
	NLnet XiGuaPi V3 TFT Screen
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	(cd $(PKG_BUILD_DIR) && \
	 git clone --depth 1 -b release/v9.3 https://github.com/lvgl/lvgl.git lvgl)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/zz
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/zz_xgp_screen $(1)/usr/zz/zz_xgp_screen
	$(INSTALL_BIN) ./files/modem_info.py $(1)/usr/zz/modem_info.py
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/zz_xgp_screen.init $(1)/etc/init.d/zz_xgp_screen
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || {
	/etc/init.d/zz_xgp_screen enable
	/etc/init.d/zz_xgp_screen start
}
endef

define Package/$(PKG_NAME)/prerm
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || {
	/etc/init.d/zz_xgp_screen stop
	/etc/init.d/zz_xgp_screen disable
}
endef

$(eval $(call BuildPackage,$(PKG_NAME)))