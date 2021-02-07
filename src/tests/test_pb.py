import os
import gzip
import unittest
import struct

import deviceapps_pb2
import pb

MAGIC = 0xFFFFFFFF
DEVICE_APPS_TYPE = 1
TEST_FILE = "test.pb.gz"
HEADER_FORMAT = 'IHH'
HEADER_LEN = 8


class TestPB(unittest.TestCase):
    deviceapps = [
        {"device": {"type": "idfa", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7c"},
         "lat": 67.7835424444, "lon": -22.8044005471, "apps": [1, 2, 3, 4]},
        {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "lat": 42, "lon": -42, "apps": [1, 2]},
        {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "lat": 42, "lon": -42, "apps": []},
        {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "apps": [1]},
    ]

    def tearDown(self):
        os.remove(TEST_FILE)

    def read_header(self, binary_stream):
        data = binary_stream.read(HEADER_LEN)
        if len(data) < HEADER_LEN:
            return
        return struct.unpack(HEADER_FORMAT, data)

    def test_write(self):
        bytes_written = pb.deviceapps_xwrite_pb(self.deviceapps, TEST_FILE)
        self.assertTrue(bytes_written > 0)
        with gzip.open(TEST_FILE) as fd:
            item_index = 0
            while True:
                header = self.read_header(fd)
                if not header:
                    break
                magic, type_, next_item_len = header
                self.assertEqual(magic, MAGIC)
                self.assertEqual(type_, DEVICE_APPS_TYPE)
                self.assertTrue(next_item_len >= 0)

                reference_item = self.deviceapps[item_index]
                device_apps = deviceapps_pb2.DeviceApps()

                data = fd.read(next_item_len)
                device_apps.ParseFromString(data)

                self.assertEqual(device_apps.device.id, reference_item['device']['id'])
                self.assertEqual(device_apps.device.type, reference_item['device']['type'])
                self.assertEqual(device_apps.apps, reference_item['apps'])
                self.assertEqual(device_apps.lat, reference_item.get('lat', 0))
                self.assertEqual(device_apps.lon, reference_item.get('lon', 0))

                item_index += 1
            self.assertEqual(item_index, len(self.deviceapps))

    @unittest.skip("Optional problem")
    def test_read(self):
        pb.deviceapps_xwrite_pb(self.deviceapps, TEST_FILE)
        for i, d in enumerate(pb.deviceapps_xread_pb(TEST_FILE)):
            self.assertEqual(d, self.deviceapps[i])
