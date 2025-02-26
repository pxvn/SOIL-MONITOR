// Simple relay test code for NodeMCU
// Connect relay to D1 (GPIO5) and test both active HIGH and LOW modes

const int RELAY_PIN = D1;      // GPIO5
const int LED_PIN = LED_BUILTIN; // Built-in LED for visual feedback
bool RELAY_ACTIVE_LOW = true;   // Change to false if your relay triggers on HIGH

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    
    // Start with relay OFF
    digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
    digitalWrite(LED_PIN, HIGH); // LED off (NodeMCU LED is active LOW)
    
    Serial.println("\nRelay Test Program");
    Serial.println("------------------");
    Serial.printf("Relay Pin: D1 (GPIO%d)\n", RELAY_PIN);
    Serial.printf("Relay Mode: Active %s\n", RELAY_ACTIVE_LOW ? "LOW" : "HIGH");
    Serial.println("Program will toggle relay every 2 seconds");
    Serial.println("Watch for relay click and LED changes");
}

void loop() {
    // Turn Relay ON
    Serial.println("\n→ Turning Relay ON");
    digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? LOW : HIGH);
    digitalWrite(LED_PIN, LOW); // LED on
    printStatus();
    delay(2000);
    
    // Turn Relay OFF
    Serial.println("\n→ Turning Relay OFF");
    digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
    digitalWrite(LED_PIN, HIGH); // LED off
    printStatus();
    delay(2000);
}

void printStatus() {
    Serial.printf("Relay Pin State: %s\n", digitalRead(RELAY_PIN) ? "HIGH" : "LOW");
    Serial.printf("LED Pin State: %s\n", digitalRead(LED_PIN) ? "HIGH" : "LOW");
    Serial.println("Check if you hear relay click and see LED change");
} 