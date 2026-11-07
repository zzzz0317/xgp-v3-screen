# xgp-v3-screen

OpenWrt TFT LCD Screen UI for **NLnet XiGuaPi Board V3**, powered by [LVGL 9](https://lvgl.io/) and Python telemetry.

This project displays system metrics, network details, internet speed throughput, and cellular modem status on the built-in 320x172 TFT LCD screen of the XiGuaPi V3 device.

---

## Screen Displays & Looping Flow (Primary Standby Mode)

The interface operates in a continuous automated carousel loop designed around real-time internet throughput monitoring:

```
[System Power On]
       │
       ▼
[Boot & Splash Screen] (~3 seconds, XiGuaPi Branding)
       │
       ▼
┌────────────────────────────────────────────────────────┐
│  1. SPEED METER SCREEN (UBIQUITI AMPLIFI HD STYLE)     │
│     PRIMARY STANDBY DURATION: 3 MINUTES (180 SECONDS)  │
│     • Download Bar (Cyan) & Upload Bar (Purple)        │
│     • Real-Time Throughput with Direction Arrows (↓/↑) │
│     • Session Bandwidth Odometer (MB / GB)             │
│     • Online ARP Clients & Peak Download Speed (Mbps)  │
└────────────────────────────────────────────────────────┘
       │
       ▼ (After 3 minutes)
┌────────────────────────────────────────────────────────┐
│  2. DEFAULT STATUS CAROUSEL (5 SECONDS PER SCREEN)     │
│     • (1/5) System Info       (5 seconds)              │
│     • (2/5) System Status     (5 seconds)              │
│     • (3/5) Network Info      (5 seconds)              │
│     • (4/5) Modem Info        (5 seconds)              │
│     • (5/5) Modem Signal      (5 seconds)              │
└────────────────────────────────────────────────────────┘
       │
       └────────► LOOPS BACK TO SPEED METER (3 MINUTES)
                  (Repeats continuously)
```

### 1. Speed Meter Screen (AmpliFi HD Style — 3 Minutes Per Cycle)
Inspired by the Ubiquiti AmpliFi HD Mesh Home Router (AFi-R) and adapted for the 320x172 landscape TFT LCD:
* **Real-Time Download Speed**: Large bold cyan digits and a glowing cyan horizontal meter bar at the top.
* **Real-Time Upload Speed**: Large bold purple digits and a glowing royal purple horizontal meter bar at the bottom.
* **Direction Indicator Arrows**: Downward arrow (↓) for download and upward arrow (↑) for upload.
* **Session Bandwidth Odometer**: Accumulated download and upload data transferred during the session (formatted in MB / GB).
* **Online Clients & Peak Speed**: Active client count from the router's ARP table, along with the all-time peak download rate recorded.
* **Micro Status**: Currently monitored interface (`br-lan • 1 Gbps`) and active connection tracking count (`nf_conntrack`).

> **Tip:** The standby duration is fully customizable via the `XGP_SPEED_STANDBY_TIME` environment variable (in seconds, default: `180`).

---

### 2. Built-in System & Modem Screens (5 Seconds Each)
After the 3-minute Speed Meter standby duration elapses, the display automatically cycles through the 5 built-in status screens (5 seconds each):

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

*(From Modem Signal 5/5, the screen automatically transitions back to the Speed Meter for another 3 minutes).*

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
   # Select:
   # -> Kernel modules -> Video Support -> kmod-fb-tft-gc9307
   # -> Utilities -> xgp-v3-screen
   ```

3. Compile the package:
   ```bash
   make package/xgp-v3-screen/compile V=s
   ```

4. Transfer and install the `.ipk` on your XiGuaPi board:
   ```bash
   opkg install /tmp/xgp-v3-screen*.ipk
   /etc/init.d/zz_xgp_screen enable
   /etc/init.d/zz_xgp_screen start
   ```

---

## Configuration & Environment Variables

You can customize runtime behavior using environment variables in OpenWrt:
* `LV_LINUX_FBDEV_DEVICE`: Path to the framebuffer device (defaults to `/dev/fb0` or `/dev/fb1`).
* `XGP_SPEED_IFACE`: Network interface monitored by the Speed Meter (defaults to `br-lan`).
* `XGP_SPEED_STANDBY_TIME`: Duration of the Speed Meter screen in seconds per cycle (defaults to `180` = 3 minutes).

---

## License

GPL-3.0-or-later. See [LICENSE](LICENSE) for details.
