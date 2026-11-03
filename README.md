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
   - Modem IP address (`wwan` / `rmnet_mhi` interfaces)
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

### 2. Kernel & Display Drivers
* Framebuffer driver for the LCD: `kmod-fb-tft-gc9307` (`/dev/fb0` or `/dev/fb1`)
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

1. Clone this repository into your OpenWrt build environment's `package` directory:
   ```bash
   cd openwrt
   git clone https://github.com/ikhsanh/xgp-v3-screen.git package/xgp-v3-screen
   ```

2. Select the package in `menuconfig`:
   ```bash
   make menuconfig
   # Utilities --->
   #   <*> xgp-v3-screen
   ```

3. Compile the package:
   ```bash
   make package/xgp-v3-screen/compile V=s
   ```

### Installing on the Device

Copy the compiled `.ipk` package to your router and install:
```bash
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

## License

This project is licensed under the **GPL-3.0** License.
