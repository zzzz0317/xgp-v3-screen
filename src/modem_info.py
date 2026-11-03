# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 zzzz0317

import json
import os
import re
import subprocess

try:
    from isp_list import lookup_isp
except ImportError:
    try:
        from .isp_list import lookup_isp
    except ImportError:
        def lookup_isp(plmn, default=""):
            return default

def parse_int_safe(val, default=0):
    if val is None:
        return default
    if isinstance(val, (int, float)):
        return int(val)
    match = re.search(r'[-+]?\d+', str(val))
    if match:
        return int(match.group(0))
    return default

def parse_float_safe(val, default=None):
    if val is None:
        return default
    if isinstance(val, (int, float)):
        return float(val)
    match = re.search(r'[-+]?\d+(?:\.\d+)?', str(val))
    if match:
        return float(match.group(0))
    return default

def parse_5gmodem_data(data):
    if not isinstance(data, dict) or not data:
        return None

    nested_dicts = [v for v in data.values() if isinstance(v, dict)]

    def get_any(*keys, default=None):
        for k in keys:
            if k in data and data[k] not in (None, ""):
                return data[k]
            for nd in nested_dicts:
                if k in nd and nd[k] not in (None, ""):
                    return nd[k]
        return default

    # Revision / Modem Model
    modem_dict = data.get('modem', {}) if isinstance(data.get('modem'), dict) else {}
    revision = (
        data.get('model') or
        data.get('revision') or
        data.get('firmware') or
        data.get('device') or
        modem_dict.get('name') or
        modem_dict.get('model') or
        (data.get('modem') if isinstance(data.get('modem'), str) else None)
    )
    if not revision or isinstance(revision, dict):
        revision = "unknown"
    revision = str(revision).strip()

    # Temperature
    raw_temp = get_any('temperature', 'temp', 'modem_temp', 'cputemp')
    if raw_temp is not None:
        temp_num = parse_float_safe(raw_temp)
        if temp_num is not None:
            temperature = f"{temp_num:.2f}℃"
        else:
            temperature = str(raw_temp).strip()
    else:
        temperature = "unknown"

    # Voltage
    raw_volt = get_any('voltage', 'volt')
    if raw_volt is not None:
        volt_num = parse_int_safe(raw_volt)
        if volt_num > 100:
            voltage = f"{volt_num} mV"
        elif volt_num > 0:
            voltage = f"{volt_num} V"
        else:
            voltage = str(raw_volt).strip()
    else:
        voltage = "-"

    # Connection Status
    connect_status = str(get_any('connect_status', 'status', 'state', 'connection', 'connected', default='connected')).strip()
    if connect_status.lower() in ('1', 'true', 'online'):
        connect_status = 'connected'

    # SIM Status
    sim_status = str(get_any('sim_status', 'sim', 'simstate', default='ready')).strip()

    # Operator / ISP
    raw_isp = get_any('operator_name', 'operator', 'carrier', 'isp', 'provider')
    plmn = str(get_any('plmn', default='')).strip()
    if not plmn:
        mcc = str(get_any('mcc', default='')).strip()
        mnc = str(get_any('mnc', default='')).strip()
        if mcc and mnc:
            plmn = f"{mcc}{mnc}"

    isp = "-"
    if plmn:
        isp = lookup_isp(plmn) or plmn
    elif raw_isp:
        if isinstance(raw_isp, str):
            isp = lookup_isp(raw_isp) or raw_isp
        elif isinstance(raw_isp, dict):
            raw_plmn = raw_isp.get('plmn') or f"{raw_isp.get('mcc', '')}{raw_isp.get('mnc', '')}"
            isp = lookup_isp(raw_plmn) or raw_isp.get('name') or "-"

    # Network Mode
    network_mode = str(get_any('mode', 'network_mode', 'netmode', 'tech', 'rat', default='unknown')).strip()
    if network_mode.endswith(" Mode"):
        network_mode = network_mode[:-5]

    # CQI
    cqi = str(get_any('cqi', default='-')).strip()

    # AMBR / Rates
    ambr = str(get_any('ambr', default='-')).strip()
    if ambr == '-':
        rates = get_any('rates', 'speed')
        if isinstance(rates, dict):
            rx = rates.get('rx', '-')
            tx = rates.get('tx', '-')
            ambr = f"{rx}/{tx}"

    # Signals: Signal 0 = RSRQ, Signal 1 = RSRP, Signal 2 = SINR
    raw_rsrq = get_any('rsrq')
    raw_rsrp = get_any('rsrp')
    raw_sinr = get_any('sinr', 'snr')

    signals = []
    # Signal 0 (RSRQ): range -20 to -3
    if raw_rsrq is not None:
        val = parse_int_safe(raw_rsrq)
        signals.append(("RSRQ", val, -20, -3, f"{val}/-3dB"))
    else:
        signals.append(("-", 0, 0, 0, "-"))

    # Signal 1 (RSRP): range -140 to -44
    if raw_rsrp is not None:
        val = parse_int_safe(raw_rsrp)
        signals.append(("RSRP", val, -140, -44, f"{val}/-44dBm"))
    else:
        signals.append(("-", 0, 0, 0, "-"))

    # Signal 2 (SINR): range -10 to 30
    if raw_sinr is not None:
        val = parse_int_safe(raw_sinr)
        signals.append(("SINR", val, -10, 30, f"{val}/30dB"))
    else:
        signals.append(("-", 0, 0, 0, "-"))

    result_txt = [
        f"revision:{revision}",
        f"temperature:{temperature}",
        f"voltage:{voltage}",
        f"connect:{connect_status}",
        f"sim:{sim_status}",
        f"isp:{isp}",
        f"cqi:{cqi}",
        f"ambr:{ambr}",
        f"networkmode:{network_mode}",
    ]
    for i, (name, val, min_v, max_v, unit) in enumerate(signals[:3]):
        result_txt.append(f"signal{i}name:{name}")
        result_txt.append(f"signal{i}value:{val}")
        result_txt.append(f"signal{i}min:{min_v}")
        result_txt.append(f"signal{i}max:{max_v}")
        result_txt.append(f"signal{i}unit:{unit}")

    return "\n".join(result_txt)

def get_5gmodem_info():
    candidates = [
        '/tmp/5gmodem/tele.json',
        '/tmp/5gmodem/data.json',
        '/tmp/5gmodem/status.json',
        '/tmp/3ginfo.json',
        '/tmp/modem.json',
    ]
    for path in candidates:
        if os.path.isfile(path):
            try:
                with open(path, 'r', encoding='utf-8', errors='ignore') as f:
                    data = json.load(f)
                    info = parse_5gmodem_data(data)
                    if info:
                        return info
            except Exception:
                continue
    return None

def get_qmodem_info():
    try:
        result = subprocess.run(
            ['/usr/libexec/rpcd/modem_ctrl', 'call', 'info'],
            capture_output=True,
            text=True,
            check=True
        )
        data = json.loads(result.stdout)
    except (subprocess.CalledProcessError, json.JSONDecodeError, FileNotFoundError, OSError):
        return None

    target_keys = {
        'revision', 'temperature', 'voltage', 'connect_status',
        'SIM Status', 'sim_status', 'ISP', 'CQI UL', 'CQI DL', 'AMBR UL', 'AMBR DL', 'network_mode',
        "MMC", "MCC", "MNC"
    }

    result = {}
    result_progress = {}
    result_txt = []

    data = data.get("info", [])
    if not isinstance(data, list) or len(data) == 0:
        return None
    data = data[0]
    data = data.get("modem_info", [])
    if not isinstance(data, list) or len(data) == 0:
        return None

    for d in data:
        if d.get("key") in target_keys:
            result[d["key"]] = str(d["value"]).strip()
        elif d.get("type") == "progress_bar" and d.get("class") == "Cell Information":
            result_progress[str(d["key"]).replace(" ", "")] = {
                "value": str(d["value"]),
                "min_value": str(d["min_value"]),
                "max_value": str(d["max_value"]),
                "unit": str(d["unit"]),
            }

    default_unknown_value_relative_to_sim_status = "-"
    sim_status = result.get('SIM Status', result.get('sim_status', 'unknown'))
    if sim_status in ["miss"]:
        default_unknown_value_relative_to_sim_status = "No SIM"

    if result.get("network_mode", "unknown").endswith(" Mode"):
        result["network_mode"] = result["network_mode"][:-5]

    if result.get("ISP", "????") == "????":
        plmn = f"{result.get('MMC', result.get('MCC', ''))}{result.get('MNC', default_unknown_value_relative_to_sim_status)}"
        isp_name = lookup_isp(plmn)
        result["ISP"] = isp_name if isp_name else plmn
    else:
        isp_name = lookup_isp(result["ISP"])
        if isp_name:
            result["ISP"] = isp_name

    if result.get('CQI DL', '') == "":
        result['CQI DL'] = "-"
    if result.get('CQI UL', '') == "":
        result['CQI UL'] = "-"
    if result['CQI DL'] == "-" and result['CQI UL'] == "-":
        result['CQI'] = default_unknown_value_relative_to_sim_status
    else:
        result['CQI'] = f"DL {result.get('CQI DL')} UL {result.get('CQI UL')}"

    if result.get('AMBR DL', '') == "":
        result['AMBR DL'] = "-"
    if result.get('AMBR UL', '') == "":
        result['AMBR UL'] = "-"
    if result['AMBR DL'] == "-" and result['AMBR UL'] == "-":
        result['AMBR'] = default_unknown_value_relative_to_sim_status
    else:
        result['AMBR'] = f"{result.get('AMBR DL')}/{result.get('AMBR UL')}"

    result_txt.append(f"revision:{result.get('revision', 'unknown')}")
    result_txt.append(f"temperature:{result.get('temperature', 'unknown')}")
    result_txt.append(f"voltage:{result.get('voltage', 'unknown')}")
    result_txt.append(f"connect:{result.get('connect_status', default_unknown_value_relative_to_sim_status)}")
    result_txt.append(f"sim:{sim_status}")
    result_txt.append(f"isp:{result.get('ISP', default_unknown_value_relative_to_sim_status)}")
    result_txt.append(f"cqi:{result['CQI']}")
    result_txt.append(f"ambr:{result['AMBR']}")
    result_txt.append(f"networkmode:{result.get('network_mode', default_unknown_value_relative_to_sim_status)}")

    result_progress_keys = list(result_progress.keys())

    for i in range(3):
        try:
            d = result_progress_keys.pop(0)
            result_txt.append(f"signal{i}name:{d}")
            result_txt.append(f"signal{i}value:{result_progress[d]['value']}")
            result_txt.append(f"signal{i}min:{result_progress[d]['min_value']}")
            result_txt.append(f"signal{i}max:{result_progress[d]['max_value']}")
            result_txt.append(f"signal{i}unit:{result_progress[d]['value']}/{result_progress[d]['max_value']}{result_progress[d]['unit']}")
        except IndexError:
            result_txt.append(f"signal{i}name:-")
            result_txt.append(f"signal{i}value:0")
            result_txt.append(f"signal{i}min:0")
            result_txt.append(f"signal{i}max:0")
            result_txt.append(f"signal{i}unit:-")

    return "\n".join(result_txt)

def get_modem_info():
    # 1. Try luci-app-5gmodem (reads tele.json / status file from /tmp)
    info = get_5gmodem_info()
    if info:
        return info

    # 2. Try QModem (rpcd modem_ctrl call info)
    info = get_qmodem_info()
    if info:
        return info

    return None

if __name__ == '__main__':
    info = get_modem_info()
    if info:
        print(info)
