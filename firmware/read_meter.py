#!/usr/bin/env python3
import subprocess
import struct
import time

PYOCD = "/home/htynkn/.platformio/penv/bin/pyocd"

def read_telemetry():
    cmd = [PYOCD, "cmd", "-t", "gd32e230c8", "-c", "read32 0x20000058 116"]
    res = subprocess.run(cmd, capture_output=True, text=True)
    words = []
    for line in res.stdout.splitlines():
        if ":" in line and "|" in line:
            parts = line.split(":")
            hex_part = parts[1].split("|")[0].strip()
            for w in hex_part.split():
                if len(w) == 8:
                    try:
                        words.append(int(w, 16))
                    except ValueError:
                        pass
    if len(words) < 14:
        return None

    raw = struct.pack(f"<{len(words)}I", *words)
    magic, uptime_ms, loop_count = struct.unpack("<III", raw[0:12])
    if magic != 0x47443332:
        return None

    oled_online, ina_online, ui_page, _ = struct.unpack("<BBBB", raw[12:16])
    v, i, p, v_shunt = struct.unpack("<ffff", raw[16:32])
    cap_mah, eng_mwh, v_max, v_min, i_max = struct.unpack("<fffff", raw[32:52])
    log = raw[52:116].split(b'\x00')[0].decode('utf-8', errors='ignore')

    return {
        "uptime_s": uptime_ms / 1000.0,
        "loop_count": loop_count,
        "oled_online": oled_online,
        "ina_online": ina_online,
        "ui_page": ui_page,
        "voltage_v": v,
        "current_a": i,
        "power_w": p,
        "shunt_mv": v_shunt,
        "cap_mah": cap_mah,
        "eng_mwh": eng_mwh,
        "v_max": v_max,
        "v_min": v_min,
        "i_max": i_max,
        "log": log
    }

def main():
    t = read_telemetry()
    if not t:
        print("Failed to read telemetry from GD32.")
        return

    print("=" * 60)
    print("        GD32 电流电压表系统运行与遥测监控")
    print("=" * 60)
    print(f"运行时间: {t['uptime_s']:.1f} 秒 (循环次数: {t['loop_count']})")
    print(f"OLED 状态: {'【已上线点亮 (0x3C)】' if t['oled_online'] else '【未检测到连接 (0x3C)】'}")
    print(f"INA226状态: {'【已上线正常 (0x40)】' if t['ina_online'] else '【未连接或异常】'}")
    print(f"当前界面: 第 {t['ui_page'] + 1} 页")
    print("-" * 60)
    print(f"总线电压: {t['voltage_v']:.3f} V")
    print(f"回路电流: {t['current_a']:.4f} A ({t['current_a']*1000:.1f} mA)")
    print(f"实时功率: {t['power_w']:.3f} W")
    print(f"分流压降: {t['shunt_mv']:.3f} mV (采样电阻 50mΩ)")
    print("-" * 60)
    print(f"累计电量: {t['cap_mah']:.2f} mAh")
    print(f"累计能量: {t['eng_mwh']:.2f} mWh")
    print(f"最高电压: {t['v_max']:.3f} V | 最低电压: {t['v_min']:.3f} V")
    print(f"峰值电流: {t['i_max']:.3f} A")
    print(f"底层日志: {t['log']}")
    print("=" * 60)

if __name__ == "__main__":
    main()
