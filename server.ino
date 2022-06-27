const size_t SCAN_RESULT_MAX = 30;
BleScanResult scanResults[SCAN_RESULT_MAX];
BleCharacteristic peerLed;
BleCharacteristic blindsChar;

bool send_request = false;
bool send_request_blinds = false;

uint8_t turn_amount[1];

Vector<BleAddress> connections;
Vector<BleAddress> blinds_connections;

uint8_t led_command[1];
bool scan_network = true;

const int MAX_TRIES = 10;

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

int blinds(String command)
{
    turn_amount[0] = command.toInt();

    send_request_blinds = true;
    return send_request_blinds;
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
            connections.append(scanResults[i].address);
        } else if (!blinds_connections.contains(scanResults[i].address) && len == 5 && buf[0] == 0x63 && buf[1] == 0x61 && buf[2] == 0x72 && buf[3] == 0x6c && buf[4] == 0x62) {
            blinds_connections.append(scanResults[i].address);
        }
    }
    
    int total_registerd_peers = connections.size() + blinds_connections.size();

    if (total_registerd_peers == 5)
    {
        Particle.publish(String::format("Finished registering %d peers", total_registerd_peers));
        scan_network = false;
    }
    else
    {
        Particle.publish(String::format("Retrying - only registered %d peers", total_registerd_peers));
    }
}

void process_request()
{
    for (int i = 0; i < connections.size(); i++)
    {
        BlePeerDevice peer;
        int tries = 0;
        while (!(peer = BLE.connect(connections[i])).connected() && tries < MAX_TRIES)
        {
            tries++;
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

void process_blinds_request()
{
    for (int i = 0; i < blinds_connections.size(); i++)
    {
        BlePeerDevice peer;
        int tries = 0;
        while (!(peer = BLE.connect(blinds_connections[i])).connected() && tries < MAX_TRIES)
        {
            tries++;
        }
        if (peer.getCharacteristicByUUID(blindsChar, "b4250402-fb4b-4746-b2b0-93f0e61122c6"))
        {
            Particle.publish(String::format("Found blinds - sending command - turning by %d", turn_amount[0]));
            blindsChar.setValue(turn_amount, sizeof(turn_amount));
        }
        peer.disconnect();
    }
    send_request_blinds = false;
}

void setup()
{
    Particle.function("led", ledToggle);
    Particle.function("blinds", blinds);
    RGB.brightness(5);
}

void loop()
{
    if (scan_network)
    {
        connect();
    }
    if (send_request)
    {
        process_request();
    }
    if (send_request_blinds) {
        process_blinds_request();
    }
}
