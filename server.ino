const size_t SCAN_RESULT_MAX = 30;
BleScanResult scanResults[SCAN_RESULT_MAX];
BleCharacteristic peerRealTimeSensorCharacteristic;

bool send_request = false;
Vector<BlePeerDevice> peers;

int ledToggle(String command)
{
	send_request = true;
	return send_request;
}

// turn off wifi and set led pin
void setup()
{
	Serial.begin(9600);
	Particle.function("led", ledToggle);

	delay(10000);
	BLE.setScanTimeout(50);
	int count = BLE.scan(scanResults, SCAN_RESULT_MAX);
	Serial.printlnf("%d ble found", count);
	for (int i = 0; i < count; i++)
	{
		uint8_t buf[BLE_MAX_ADV_DATA_LEN];
		size_t len;
		len = scanResults[i].advertisingData.get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA,
												 buf, BLE_MAX_ADV_DATA_LEN);

		if (len == 5 && buf[0] == 0x63 && buf[1] == 0x61 && buf[2] == 0x72 && buf[3] == 0x6c && buf[4] == 0x61)
		{
			// discovered a peer
			BlePeerDevice peer = BLE.connect(scanResults[i].address);
			while (!peer.connected())
			{
			}
			peers.append(peer);
			Serial.printlnf("successfully connected to %02X:%02X:%02X:%02X:%02X:%02X!",
							scanResults[i].address[0], scanResults[i].address[1], scanResults[i].address[2],
							scanResults[i].address[3], scanResults[i].address[4], scanResults[i].address[5]);
		}
	}
	Serial.printlnf("Finished registering %d peers", peers.size());
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
	if (send_request)
	{
		for (int i = 0; i < peers.size(); i++)
		{
			if (peers[i].getCharacteristicByUUID(peerRealTimeSensorCharacteristic, "b4250401-fb4b-4746-b2b0-93f0e61122c6"))
			{
				Serial.printlnf("Found LiveSensorDataCharacteristic - hooking up NOTIFY callback");
				uint8_t val[1];
				val[0] = 0xBB;
				peerRealTimeSensorCharacteristic.setValue(val, sizeof(val));
			}
		}
		send_request = false;
	}
}
