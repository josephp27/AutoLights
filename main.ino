int led1 = D6;
bool is_on = false;

void setup() {
    // turn off the main led to save battery
    RGB.brightness(0);
    
    pinMode(led1, OUTPUT);
    Particle.function("led", ledToggle);
}

void loop() {
    //empty for now
}


int ledToggle(String command) {
    is_on ^= 1;
    
    if (is_on == true) {
        digitalWrite(led1, HIGH);
        return 1;
    }

    digitalWrite(led1, LOW);
    return 0;
}
