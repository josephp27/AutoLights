SYSTEM_MODE(MANUAL);

// UUIDs for service + characteristics
const char *serviceUuid = "b4250400-fb4b-4746-b2b0-93f0e61122c6"; //service
const char *ledUuid = "b4250401-fb4b-4746-b2b0-93f0e61122c6";	  //led
bool is_on = false;

// Static function for handling Bluetooth Low Energy callbacks
static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{

	// We're only looking for one byte
	if (len != 1)
	{
		return;
	}

	if (data[0] == 0xFF)
	{
		is_on = true;
	}
	else if (data[0] == 0x00)
	{
		is_on = false;
	}
	else
	{
		is_on ^= 1;
	}

	digitalWrite(D6, is_on);
}

// turn off wifi and set led pin
void setup()
{
	WiFi.off();
	pinMode(D6, OUTPUT);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
	// Set up characteristics
	BleCharacteristic ledCharacteristic("led", BleCharacteristicProperty::WRITE_WO_RSP, ledUuid, serviceUuid, onDataReceived, (void *)ledUuid);

	// Add the characteristics
	BLE.addCharacteristic(ledCharacteristic);

	uint8_t buf[BLE_MAX_ADV_DATA_LEN];
	size_t offset = 0;

	// Company ID (0xffff internal use/testing)
	buf[offset++] = 0x63;
	buf[offset++] = 0x61;
	buf[offset++] = 0x72;
	buf[offset++] = 0x6c;
	buf[offset++] = 0x61;

	// Advertising data
	BleAdvertisingData advData;
	advData.appendCustomData(buf, offset);

	// Start advertising!
	BLE.advertise(&advData);

	delay(10000);

	SystemSleepConfiguration config;
	config.mode(SystemSleepMode::ULTRA_LOW_POWER)
		.ble();

	SystemSleepResult res = System.sleep(config);
}
