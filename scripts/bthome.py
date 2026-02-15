# import asyncio
# import struct

# from bleak import BleakScanner
# from bleparser import BleParser

# timeout_seconds = 2000
# address_to_look_for = 'FB:D6:DA:31:25:F8'
# service_id_to_look_for = '0000d2fc-0000-1000-8000-00805f9b34fb'


# class MyScanner:
#     def __init__(self):
#         self._scanner = BleakScanner(detection_callback=self.detection_callback)
#         self.parser = BleParser()

#         self.scanning = asyncio.Event()
#     def detection_callback(self, device, advertisement_data):
#         # Looking for:
#         # AdvertisementData(service_data={
#         # '0000feaa-0000-1000-8000-00805f9b34fb': b'\x00\xf6\x00\x00\x00Jupiter\x00\x00\x00\x00\x00\x0b'},
#         # service_uuids=['0000feaa-0000-1000-8000-00805f9b34fb'])
#         if device.address == address_to_look_for:
#             byte_data = advertisement_data.service_data.get(service_id_to_look_for)
#             sensor_msg, tracker_msg = self.parser.parse_raw_data(byte_data)
#             num_to_test, = struct.unpack_from('<I', byte_data, 0)
#             if num_to_test == 62976:
#                 print('\t\tDevice found so we terminate')
#                 self.scanning.clear()

#     async def run(self):
#         await self._scanner.start()
#         self.scanning.set()
#         end_time = loop.time() + timeout_seconds
#         while self.scanning.is_set():
#             if loop.time() > end_time:
#                 self.scanning.clear()
#                 print('\t\tScan has timed out so we terminate')
#             await asyncio.sleep(0.1)
#         await self._scanner.stop()


# if __name__ == '__main__':
#     my_scanner = MyScanner()
#     loop = asyncio.get_event_loop()
#     loop.run_until_complete(my_scanner.run())

import asyncio
from datetime import datetime
from bleak import BleakScanner
from bthome_ble.parser import BTHomeBluetoothDeviceData, BluetoothServiceInfoBleak

def pretty_print_sensors(sensor_data):
    print("="*30)
    for key, value in sensor_data.items():
        print(f"🔹 {value.name}: {value.native_value}")
    print("="*30)

def is_bthome_device(advertisement_data):
    """Check if the device follows the BTHome BLE format (UUID: 0xFCD2)."""
    return any(key.startswith("0000fcd2") for key in advertisement_data.service_data.keys())

async def scan_bthome_devices():
    print("Scanning for BTHome devices...")

    def detection_callback(device, advertisement_data):
        if is_bthome_device(advertisement_data):
            dev = BTHomeBluetoothDeviceData()
            data = BluetoothServiceInfoBleak(
                name=advertisement_data.local_name,
                address=device.address,
                rssi=device.rssi,
                manufacturer_data={},
                service_data=advertisement_data.service_data,
                service_uuids=[],
                source="",
                device=None,
                advertisement=None,
                connectable=False,
                time=datetime.now().timestamp(),
                tx_power=None,
            )
            dev.update(data)
            print(f"\n📡 Found BTHome Device: {device.name or 'Unknown'} - {device.address}")
            pretty_print_sensors(dev._sensor_values)

    scanner = BleakScanner(detection_callback=detection_callback)
    await scanner.start()
    await asyncio.sleep(100)  # Scan for 10 seconds
    await scanner.stop()
    print("Scan complete.")

# Run the scanner
asyncio.run(scan_bthome_devices())
