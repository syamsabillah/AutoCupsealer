#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ezButton.h>

#define SENSOR  13
#define Relay_Valve 15
#define Relay_LED 2
#define Relay_Pompa 23
#define SDA_LCD 21
#define SCL_LCD 22

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR, INPUT_PULLUP);
  pinMode(Relay_Valve, OUTPUT);
  pinMode(Relay_LED, OUTPUT);
  pinMode(Relay_Pompa, OUTPUT);

  digitalWrite(Relay_Valve, LOW); // Valve closed
  digitalWrite(Relay_LED, LOW);  // LED off
  digitalWrite(Relay_Pompa, LOW); // Pump off

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Send Volume via");
  lcd.setCursor(0, 1);
  lcd.print("Serial (mL)");

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}

void loop() {
  // Check for serial input
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Remove any extra whitespace or newline characters

    // Convert input to float
    float targetVolume = input.toFloat();

    if (targetVolume > 0) { // Ensure a valid target volume
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dispensing...");
      Serial.println("Dispensing started...");
      Serial.print("Target Volume: ");
      Serial.print(targetVolume);
      Serial.println(" mL");

      // Flow rate in mL/s (3.5 L/min = 58.33 mL/s)
      const float flowRate1 = 58.33;
      unsigned long runTime = (targetVolume / flowRate1) * 1000; // Calculate run time in ms

      // Start pump and valve
      digitalWrite(Relay_Valve, HIGH); // Open valve
      digitalWrite(Relay_Pompa, HIGH); // Start pump

      unsigned long startTime = millis();

      // Dispensing loop
      while (millis() - startTime < runTime) {
        currentMillis = millis();
        if (currentMillis - previousMillis > interval) {
          pulse1Sec = pulseCount;
          pulseCount = 0;

          // Calculate flow rate
          flowRate = ((1000.0 / (currentMillis - previousMillis)) * pulse1Sec) / calibrationFactor;
          previousMillis = currentMillis;

          // Calculate total volume
          flowMilliLitres = (flowRate / 60) * 1000;
          totalMilliLitres += flowMilliLitres;

          // Print flow data
          Serial.print("Flow rate: ");
          Serial.print(int(flowRate));  // Print the integer part of the variable
          Serial.print(" L/min\t");
          Serial.print("Output Liquid Quantity: ");
          Serial.print(totalMilliLitres);
          Serial.print(" mL / ");
          Serial.print(totalMilliLitres / 1000);
          Serial.println(" L");
        }
      }

      // Stop pump and valve
      digitalWrite(Relay_Valve, LOW); // Close valve
      digitalWrite(Relay_Pompa, LOW); // Stop pump

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dispensing Done");
      lcd.setCursor(0, 1);
      lcd.print("Pulses: ");
      lcd.print(pulseCount);

      Serial.println("Dispensing complete.");
      Serial.print("Total Pulses: ");
      Serial.println(pulseCount);
    } else {
      Serial.println("Invalid input. Please enter a positive number for volume.");
    }
  }
}
