SYSTEM_MODE(MANUAL);

#include <Stepper.h>

// // UUIDs for service + characteristics
const char *serviceUuid = "abcde422-fb4b-4746-b2b0-93f0e61122c6"; // service
const char *ledUuid = "b4250402-fb4b-4746-b2b0-93f0e61122c6";     // led

const int stepsPerRevolution = 200; // change this to fit the number of steps per revolution
Stepper myStepper(stepsPerRevolution, D0, D1, D2, D3);

SystemSleepConfiguration config;

int steps_to_turn = 0;
bool on_start = true;

void turn_on()
{
  digitalWrite(D5, LOW);
  delay(100);
  digitalWrite(D5, HIGH);
  delay(1000);

  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  digitalWrite(D4, HIGH);
}

void turn_off()
{
  digitalWrite(D4, LOW);
  pinMode(D0, INPUT_PULLUP);
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);

  digitalWrite(D5, LOW);
  delay(100);
  digitalWrite(D5, HIGH);
  delay(100);
  digitalWrite(D5, LOW);
  delay(100);
  digitalWrite(D5, HIGH);
}

// Static function for handling Bluetooth Low Energy callbacks
static void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{

  // 	We're only looking for one byte
  if (len != 1)
  {
    return;
  }

  steps_to_turn = data[0] - 100;
}

void setup()
{
  // RGB.brightness(0);

  WiFi.off();

  config.mode(SystemSleepMode::ULTRA_LOW_POWER)
      .ble();

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

  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);

  myStepper.setSpeed(15);

  turn_on();
  turn_off();
}

void loop()
{
  if (on_start)
  {
    on_start = false;
    SystemSleepResult res = System.sleep(config);
  }
  if (steps_to_turn != 0)
  {
    turn_on();

    myStepper.step(steps_to_turn);

    turn_off();

    steps_to_turn = 0;
    SystemSleepResult res = System.sleep(config);
  }
}
