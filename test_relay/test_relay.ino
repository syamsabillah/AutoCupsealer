#define Relay_Valve 15
#define Relay_LED 2

void setup() {
  // Initialize the relay pins
  pinMode(Relay_Valve, OUTPUT);
  pinMode(Relay_LED, OUTPUT);
  
  // Start serial communication at 9600 baud rate
  Serial.begin(9600);
  
  // Set both relays to LOW at the beginning (off)
  digitalWrite(Relay_Valve, HIGH);
  digitalWrite(Relay_LED, HIGH);
}

void loop() {
  // Check if there is any serial input
  if (Serial.available() > 0) {
    // Read the incoming byte from serial
    char input = Serial.read();
    
    // Control relays based on the input
    switch(input) {
      case '1':  // Activate solenoid relay
        digitalWrite(Relay_Valve, HIGH);
        Serial.println("Relay_Valve Mati.");
        break;
      case '2':  // Deactivate solenoid relay
        digitalWrite(Relay_Valve, LOW);
        Serial.println("Relay_Valve Nyala.");
        break;
      case '3':  // Activate water pump relay
        digitalWrite(Relay_LED, HIGH);
        Serial.println("Relay_LED Mati.");
        break;
      case '4':  // Deactivate water pump relay
        digitalWrite(Relay_LED, LOW);
        Serial.println("Relay_LED Nyala.");
        break;
      default:
        Serial.println("Invalid input.");
        break;
    }
  }
}
