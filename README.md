# xgp-v3-screen

OpenWrt TFT LCD Screen UI for **NLnet XiGuaPi Board V3**, powered by [LVGL 9](https://lvgl.io/) and Python telemetry.

This project displays system metrics, network details, and cellular modem status on the built-in 320x172 TFT LCD screen of the XiGuaPi V3 device.

---

## Screen Displays & Features

The interface automatically cycles through 5 informative screens:

1. **System Info (1/5)**:
   - Hostname
   - OS Distribution & Release name
   - Build ID
   - Linux Kernel version

2. **System Status (2/5)**:
   - CPU Load Average (1m / 5m / 15m)
   - Memory usage (used / total in MB and GB, with percentage)
   - System Uptime (days, hours, minutes, seconds)
   - Local Time & Date

3. **Network Info (3/5)**:
   - Modem IP address (supports `wwan`, `rmnet`, `usb`, `qmi`, `mbim`, `modem`, `eth2-4`, and OpenWrt `ubus`)
   - WAN IP address
   - LAN IP address
   - Active network connections count (`nf_conntrack`)
   - Online clients / connected devices count (`ARP` table)

4. **Modem Info (4/5)**:
   - Modem Model / Firmware Revision
   - Operating Temperature
   - Voltage
   - Cellular Operator (ISP) with auto-resolved brand name
   - Network Mode (5G SA, 5G NSA, LTE, etc.)
   - AMBR / Negotiated Data Rates

5. **Modem Signal (5/5)**:
   - **RSRQ** (Reference Signal Received Quality) bar and dB value
   - **RSRP** (Reference Signal Received Power) bar and dBm value
   - **SINR** (Signal to Interference-plus-Noise Ratio) bar and dB value

---

## Requirements & Prerequisites

### 1. Modem Telemetry Backend (Required)
To display cellular modem information (Modem Info & Signal screens), you **must install**:
* **[luci-app-5gmodem](https://github.com/fildunsky/luci-app-5gmodem)** (Recommended)
  
  `xgp-v3-screen` reads real-time modem telemetry directly from `luci-app-5gmodem` (`/tmp/5gmodem/tele.json`).
  
  *(Note: It also supports [QModem](https://github.com/FUjr/QModem) as a secondary fallback).*

### 2. LCD Display Driver (Required)
* **[kmod-fb-tft-gc9307](https://github.com/zzzz0317/kmod-fb-tft-gc9307)**:
  
  Linux kernel framebuffer driver module for the GC9307 TFT LCD screen used on the XiGuaPi V3 board (provides `/dev/fb0` or `/dev/fb1`).

### 3. Screen Management Web UI (Recommended)
* **[luci-app-xgp-v3-screen](https://github.com/ikhsanh/luci-app-xgp-v3-screen)**:
  
  Companion OpenWrt LuCI web application providing:
  - Screen power toggle (Enable / Disable switch)
  - Backlight brightness slider (0 - 255)
  - Display Interval cycling (e.g. 10 minutes ON, 10 minutes OFF loop)
  - Time schedule / autoswitch (e.g. ON only between 20:00 - 23:00)

### 4. System Dependencies
* `python3`
* `libpthread`
* `libstdcpp`

---

## Worldwide Operator (ISP) Support

The application includes an extensive database (`isp_list.py`) mapping PLMN codes (MCC + MNC) to concise carrier brand names:
* **Indonesia**: Telkomsel, XL Axiata, Indosat Ooredoo Hutchison, AXIS, Smartfren, Tri (3), etc.
* **Southeast Asia**: Malaysia (Maxis, CelcomDigi, U Mobile, Yes), Singapore (Singtel, StarHub, Simba), Thailand, Philippines, Vietnam, etc.
* **Global**: Major network operators across East Asia, South Asia, Middle East, Europe, Americas, Africa, and Oceania.

---

## Installation & Build

### Building with OpenWrt SDK

1. Clone this repository and the LCD driver package into your OpenWrt build environment's `package` directory:
   ```bash
   cd openwrt
   git clone https://github.com/zzzz0317/kmod-fb-tft-gc9307.git package/kmod-fb-tft-gc9307
   git clone https://github.com/ikhsanh/xgp-v3-screen.git package/xgp-v3-screen
   ```

2. Select the packages in `menuconfig`:
   ```bash
   make menuconfig
   # Kernel modules --->
   #   Video Support --->
   #     <*> kmod-fb-tft-gc9307
   # Utilities --->
   #   <*> xgp-v3-screen
   ```

3. Compile the packages:
   ```bash
   make package/kmod-fb-tft-gc9307/compile V=s
   make package/xgp-v3-screen/compile V=s
   ```

### Installing on the Device

Copy the compiled `.ipk` packages to your router and install:
```bash
# Install kernel driver and screen UI
opkg install kmod-fb-tft-gc9307_*.ipk
opkg install xgp-v3-screen_*.ipk
```

Make sure **luci-app-5gmodem** is installed as well:
```bash
# Example installing luci-app-5gmodem
opkg install luci-app-5gmodem_*.ipk
```

### Service Control

The screen service runs automatically via `procd`:
```bash
# Check status / restart service
/etc/init.d/zz_xgp_screen status
/etc/init.d/zz_xgp_screen restart

# Enable / Disable auto-start on boot
/etc/init.d/zz_xgp_screen enable
/etc/init.d/zz_xgp_screen disable
```

---

## Credits & Upstream Source

This project is based on the original source code by **[zzzz0317/xgp-v3-screen](https://github.com/zzzz0317/xgp-v3-screen)**.

Special thanks to:
* **[zzzz0317](https://github.com/zzzz0317)** for the original `xgp-v3-screen` project and `kmod-fb-tft-gc9307` display driver.
* **[fildunsky/luci-app-5gmodem](https://github.com/fildunsky/luci-app-5gmodem)** for the cellular modem telemetry provider.

---

## License

This project is licensed under the **GPL-3.0** License.
