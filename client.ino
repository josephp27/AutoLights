SYSTEM_MODE(MANUAL);

#include <Stepper.h>

// UUIDs for service + characteristics
const char *serviceUuid = "abcdef78-fb4b-4746-b2b0-93f0e61122c6"; // service
const char *ledUuid = "b4250402-fb4b-4746-b2b0-93f0e61122c6";     // led

const int stepsPerRevolution = 200; // change this to fit the number of steps per revolution

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, D0, D1, D2, D3);

// Static function for handling Bluetooth Low Energy callbacks
static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{

  // 	We're only looking for one byte
  if (len != 1)
  {
    return;
  }
  digitalWrite(D7, HIGH);

  myStepper.step(data[0] - 100);

  delay(1000);

  digitalWrite(D7, LOW);
}

// turn off wifi and set led pin
void setup()
{
  pinMode(D7, OUTPUT);
  WiFi.off();
  myStepper.setSpeed(60);
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
  buf[offset++] = 0x62;

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
