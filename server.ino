const size_t SCAN_RESULT_MAX = 30;
BleScanResult scanResults[SCAN_RESULT_MAX];
BleCharacteristic peerLed;

bool send_request = false;
Vector<BleAddress> connections;

uint8_t led_command[1];
bool scan_network = true;
bool heartbeat = false;

Timer timer(1000 * 60 * 60, run_heart_beat);

void run_heart_beat()
{
    heartbeat = true;
}

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

void check_connection()
{
    Particle.publish("Heartbeat... Starting");
    for (int i = 0; i < connections.size(); i++)
    {
        BlePeerDevice peer = BLE.connect(connections[i]);
        if (!peer.connected())
        {
            System.reset();
        }
        peer.disconnect();
    }
    Particle.publish("Heartbeat... Passed");
    heartbeat = false;
}

void connect()
{
    BLE.setScanTimeout(50);
    int count = BLE.scan(scanResults, SCAN_RESULT_MAX);

    Particle.publish(String::format("%d ble found", count));
    for (int i = 0; i < count; i++)
    {
        uint8_t buf[BLE_MAX_ADV_DATA_LEN];
        size_t len = scanResults[i].advertisingData.get(BleAdvertisingDataType::MANUFACTURER_SPECIFIC_DATA, buf, BLE_MAX_ADV_DATA_LEN);

        if (!connections.contains(scanResults[i].address) && len == 5 && buf[0] == 0x63 && buf[1] == 0x61 && buf[2] == 0x72 && buf[3] == 0x6c && buf[4] == 0x61)
        {
            // discovered a peer
            Particle.publish(String::format("attempting connection to %02X:%02X:%02X:%02X:%02X:%02X!",
                                            scanResults[i].address[0], scanResults[i].address[1], scanResults[i].address[2],
                                            scanResults[i].address[3], scanResults[i].address[4], scanResults[i].address[5]));

            connections.append(scanResults[i].address);

            Particle.publish(String::format("successfully connected to %02X:%02X:%02X:%02X:%02X:%02X!",
                                            scanResults[i].address[0], scanResults[i].address[1], scanResults[i].address[2],
                                            scanResults[i].address[3], scanResults[i].address[4], scanResults[i].address[5]));
        }
    }

    if (connections.size() == 3)
    {
        Particle.publish(String::format("Finished registering %d peers", connections.size()));
        check_connection();
        scan_network = false;
    }
    else
    {
        Particle.publish(String::format("Retrying - only registered %d peers", connections.size()));
    }
}

void process_request()
{
    for (int i = 0; i < connections.size(); i++)
    {
        BlePeerDevice peer;
        while (!(peer = BLE.connect(connections[i])).connected())
        {
        }
        if (peer.getCharacteristicByUUID(peerLed, "b4250401-fb4b-4746-b2b0-93f0e61122c6"))
        {
            Particle.publish("Found peerLed - sending command");
            peerLed.setValue(led_command, sizeof(led_command));
        }
        peer.disconnect();
    }
    send_request = false;
}

void setup()
{
    Particle.function("led", ledToggle);
    timer.start();
    RGB.brightness(5);
}

void loop()
{
    if (heartbeat)
    {
        check_connection();
    }
    if (scan_network)
    {
        connect();
    }
    if (send_request)
    {
        process_request();
    }
}
