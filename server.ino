const size_t SCAN_RESULT_MAX = 30;
BleScanResult scanResults[SCAN_RESULT_MAX];
BleCharacteristic peerLed;

bool send_request = false;
Vector<BlePeerDevice> peers;
Vector<BleAddress> already_connected;

uint8_t led_command[1];
bool scan_network = true;

int ledToggle(String command)
{
  if (command.equals("on"))
  {
    led_command[0] = 0xFF;
  }
  else if (command.equals("off"))
  {
    led_command[0] = 0x00;
  }
  else
  {
    led_command[0] = 0xBB;
  }

  send_request = true;
  return send_request;
}

void setup()
{
  Particle.function("led", ledToggle);
  RGB.brightness(5);
}

void loop()
{
  if (scan_network)
  {
    BLE.setScanTimeout(50);
    int count = BLE.scan(scanResults, SCAN_RESULT_MAX);

    Particle.publish(String::format("%d ble found", count));
    for (int i = 0; i < count; i++)
    {
      uint8_t buf[BLE_MAX_ADV_DATA_LEN];
      size_t len = scanResults[i].advertisingData.get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, buf, BLE_MAX_ADV_DATA_LEN);

      if (!already_connected.contains(scanResults[i].address) && len == 5 && buf[0] == 0x63 && buf[1] == 0x61 && buf[2] == 0x72 && buf[3] == 0x6c && buf[4] == 0x61)
      {
        // discovered a peer
        Particle.publish(String::format("attempting connection to %02X:%02X:%02X:%02X:%02X:%02X!",
                                        scanResults[i].address[0], scanResults[i].address[1], scanResults[i].address[2],
                                        scanResults[i].address[3], scanResults[i].address[4], scanResults[i].address[5]));
        BlePeerDevice peer;
        while (!(peer = BLE.connect(scanResults[i].address)).connected())
        {
        }

        peers.append(peer);
        already_connected.append(scanResults[i].address);

        Particle.publish(String::format("successfully connected to %02X:%02X:%02X:%02X:%02X:%02X!",
                                        scanResults[i].address[0], scanResults[i].address[1], scanResults[i].address[2],
                                        scanResults[i].address[3], scanResults[i].address[4], scanResults[i].address[5]));
      }
    }

    if (peers.size() == 3)
    {
      Particle.publish(String::format("Finished registering %d peers", peers.size()));
      scan_network = false;

      //turn on all lights
      led_command[0] = 0xFF;
      send_request = true;
    }
    else
    {
      Particle.publish(String::format("Retrying - only registered %d peers", peers.size()));
      scan_network = true;
    }
  }

  if (send_request)
  {
    for (int i = 0; i < peers.size(); i++)
    {
      if (peers[i].getCharacteristicByUUID(peerLed, "b4250401-fb4b-4746-b2b0-93f0e61122c6"))
      {
        Particle.publish("Found peerLed - hooking up NOTIFY callback");
        peerLed.setValue(led_command, sizeof(led_command));
      }
    }
    send_request = false;
  }
}
