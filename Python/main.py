import asyncio
import struct
import time
import argparse # コマンドライン引数用

from bleak import BleakScanner, BleakClient
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData 

from pythonosc import udp_client # OSC送信ライブラリ

# --- BLE設定 ---
HEART_RATE_SERVICE_UUID = "0000180D-0000-1000-8000-00805F9B34FB"  # 180D
HEART_RATE_MEASUREMENT_CHAR_UUID = "00002A37-0000-1000-8000-00805F9B34FB" # 2A37
TARGET_DEVICE_NAME = "HW706"

# --- OSC設定 (グローバル変数として定義し、後で初期化) ---
osc_client = None
device_name_for_osc = "UnknownDevice" # OSCで送るデバイス名

# --- 通知ハンドラー ---
def notification_handler(sender, data):
    """
    心拍数測定値の通知を受信したときに呼び出される関数
    """
    flags = data[0]
    hr_format = (flags >> 0) & 0x01  # 0: UINT8, 1: UINT16
    
    energy_expended_present = (flags >> 3) & 0x01
    rr_interval_present = (flags >> 4) & 0x01

    heart_rate = 0
    offset = 1

    if hr_format == 0:  # UINT8
        heart_rate = data[offset]
        offset += 1
    else:  # UINT16
        heart_rate = struct.unpack("<H", data[offset:offset+2])[0]
        offset += 2

    current_timestamp_ms = int(time.time() * 1000) # 現在のタイムスタンプをミリ秒で取得

    print(f"心拍数: {heart_rate} bpm")
    
    # OSC送信
    if osc_client:
        try:
            # /avatar/parameters/HeartRate [timestamp (ms)] [デバイス名] [心拍数]
            osc_client.send_message("/avatar/parameters/HeartRate", [0, device_name_for_osc, heart_rate])
            # print(f"OSC送信: /avatar/parameters/HeartRate {current_timestamp_ms} {device_name_for_osc} {heart_rate}")
        except Exception as e:
            print(f"OSC送信エラー: {e}")

    if energy_expended_present:
        energy_expended = struct.unpack("<H", data[offset:offset+2])[0]
        offset += 2
        print(f"  消費エネルギー: {energy_expended} joules")

    if rr_interval_present:
        rr_intervals = []
        while offset < len(data):
            rr_interval = struct.unpack("<H", data[offset:offset+2])[0]
            rr_intervals.append(rr_interval / 1024.0) # 1/1024秒単位
            offset += 2
        print(f"  RR Interval: {rr_intervals} s")

# --- メイン処理 ---
async def run(args):
    global osc_client, device_name_for_osc # グローバル変数を更新できるようにする

    # OSCクライアントの初期化
    if args.osc_ip and args.osc_port:
        osc_client = udp_client.SimpleUDPClient(args.osc_ip, args.osc_port)
        print(f"OSC送信先: {args.osc_ip}:{args.osc_port}")
    else:
        print("OSC送信先IPアドレスとポートが指定されていません。OSC送信は行いません。")

    print("BLEデバイスをスキャン中...")
    
    devices_to_process = []
    try:
        discovered_data = await BleakScanner.discover(timeout=5, return_adv=True)
        for device, advertisement_data in discovered_data:
            devices_to_process.append((device, advertisement_data.service_uuids, device.name, advertisement_data.local_name))
            
    except ValueError:
        discovered_devices = await BleakScanner.discover(timeout=5)
        for device in discovered_devices:
            service_uuids_from_device = device.metadata.get("uuids", []) if hasattr(device, "metadata") else []
            devices_to_process.append((device, service_uuids_from_device, device.name, device.name))

    print("\n--- 検出された全デバイス ---")
    for device, service_uuids, device_name, local_name in devices_to_process:
        print(f"  名前: {device_name or local_name or 'N/A'}, アドレス: {device.address}, UUIDs: {service_uuids}")
    print("----------------------------\n")

    target_device = None
    for device, service_uuids, device_name, local_name in devices_to_process:
        current_device_name = device_name or local_name
        
        if current_device_name and TARGET_DEVICE_NAME.lower() in current_device_name.lower():
            print(f"ターゲットデバイス '{TARGET_DEVICE_NAME}' を発見: {current_device_name} ({device.address})")
            target_device = device
            device_name_for_osc = current_device_name # OSC送信用のデバイス名をセット
            break

    if not target_device:
        print(f"ターゲットデバイス '{TARGET_DEVICE_NAME}' が見つかりませんでした。")
        print("検出されたデバイス名を確認し、TARGET_DEVICE_NAME を修正してみてください。")
        return

    async with BleakClient(target_device.address) as client:
        if client.is_connected:
            print(f"{target_device.name} ({target_device.address}) に接続しました。")

            try:
                await client.start_notify(HEART_RATE_MEASUREMENT_CHAR_UUID, notification_handler)
                print("心拍数測定の通知を開始しました。Ctrl+Cで終了します。")
                while True:
                    await asyncio.sleep(1) 
            except Exception as e:
                print(f"エラーが発生しました: {e}")
            finally:
                await client.stop_notify(HEART_RATE_MEASUREMENT_CHAR_UUID)
                print("通知を停止しました。")
        else:
            print("接続できませんでした。")

# --- プログラム実行開始 ---
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="BLE心拍計から心拍数を取得し、OSCで送信します。")
    parser.add_argument("--osc_ip", type=str, default="127.0.0.1",
                        help="OSC送信先のIPアドレス (デフォルト: 127.0.0.1)")
    parser.add_argument("--osc_port", type=int, default=9000,
                        help="OSC送信先のポート番号 (デフォルト: 9000)")
    args = parser.parse_args()

    try:
        asyncio.run(run(args))
    except KeyboardInterrupt:
        print("\nプログラムを終了します。")