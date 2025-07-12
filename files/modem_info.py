# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2025 zzzz0317

import json
import subprocess

def get_modem_info():
    try:
        result = subprocess.run(
            ['/usr/libexec/rpcd/modem_ctrl', 'call', 'info'],
            capture_output=True,
            text=True,
            check=True
        )
        data = json.loads(result.stdout)
    except (subprocess.CalledProcessError, json.JSONDecodeError):
        return None
    target_keys = {
        'revision', 'temperature', 'voltage', 'connect_status',
        'SIM Status', 'ISP', 'CQI UL', 'CQI DL', 'AMBR UL', 'AMBR DL', 'network_mode',
        "MMC", "MCC", "MNC" # https://github.com/FUjr/QModem/pull/66
    }
    
    result = {}
    result_progress = {}
    result_txt = []
    
    data = data.get("info", [])
    if not isinstance(data, list):
        return None
    if len(data) == 0:
        return None
    data = data[0]
    data = data.get("modem_info", [])
    if not isinstance(data, list):
        return None
    if len(data) == 0:
        return None
    
    for d in data:
        if d["key"] in target_keys:
            result[d["key"]] = str(d["value"]).strip()
        elif d["type"] == "progress_bar" and d["class"] == "Cell Information":
            result_progress[str(d["key"]).replace(" ", "")] = {
                "value": str(d["value"]),
                "min_value": str(d["min_value"]),
                "max_value": str(d["max_value"]),
                "unit": str(d["unit"]),
            }
            
    if result.get("network_mode", "unknown").endswith(" Mode"):
        result["network_mode"] = result["network_mode"][:-5]
    if result.get("ISP", "????") == "????":
        result["ISP"] = f"{result.get('MMC', result.get('MCC', ''))}{result['MNC']}" # https://github.com/FUjr/QModem/pull/66
        if result["ISP"] in ["46000", "46002", "46007"]:
            result["ISP"] = "中国移动"
        elif result["ISP"] in ["46001", "46006", "46009"]:
            result["ISP"] = "中国联通"
        elif result["ISP"] in ["46003", "46005", "46011"]:
            result["ISP"] = "中国电信"
        elif result["ISP"] in ["46015"]:
            result["ISP"] = "中国广电"
        elif result["ISP"] in ["46020"]:
            result["ISP"] = "中国铁通"
        
    if result.get('CQI DL', '') == "":
        result['CQI DL'] = "-"
    if result.get('CQI UL', '') == "":
        result['CQI UL'] = "-"
    if result['CQI DL'] == "-" and result['CQI UL'] == "-":
        result['CQI'] = "unknown"
    else:
        result['CQI'] = f"DL {result.get('CQI DL')} UL {result.get('CQI UL')}"
        
    if result.get('AMBR DL', '') == "":
        result['AMBR DL'] = "-"
    if result.get('AMBR UL', '') == "":
        result['AMBR UL'] = "-"
    if result['AMBR DL'] == "-" and result['AMBR UL'] == "-":
        result['AMBR'] = "unknown"
    else:
        result['AMBR'] = f"{result.get('AMBR DL')}/{result.get('AMBR UL')}"
    
    result_txt.append(f"revision:{result.get('revision', 'unknown')}")
    result_txt.append(f"temperature:{result.get('temperature', 'unknown')}")
    result_txt.append(f"voltage:{result.get('voltage', 'unknown')}")
    result_txt.append(f"connect:{result.get('connect_status', 'unknown')}")
    result_txt.append(f"sim:{result.get('SIM Status', 'unknown')}")
    result_txt.append(f"isp:{result.get('ISP', 'unknown')}")
    result_txt.append(f"cqi:{result['CQI']}")
    result_txt.append(f"ambr:{result['AMBR']}")
    result_txt.append(f"networkmode:{result.get('network_mode', 'unknown')}")
    
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

if __name__ == '__main__':
    info = get_modem_info()
    print(info)