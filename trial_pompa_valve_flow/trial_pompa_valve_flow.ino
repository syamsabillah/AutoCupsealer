#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <FlowSensor.h>

#define Relay_Valve 15
#define Relay_LED 2
#define Relay_Pompa 23
#define type OF05ZAT
#define pin 13 // pin -> interrupt pin

#define CLK_Rotary 16
#define DT_Rotary 17
#define SW_Rotary 5
//rotary encoder
#define DIRECTION_CW  0   // clockwise direction
#define DIRECTION_CCW 1  // counter-clockwise direction
int direction = DIRECTION_CW;
int CLK_state;
int prev_CLK_state;

#define SDA_LCD 21
#define SCL_LCD 22
// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

FlowSensor Sensor(type, pin);
unsigned long timebefore = 0; // same type as millis()
float maxVolume = 1.0; // Default max volume

int rotaryPosition = 0; // Tracks the rotary encoder position
int lastRotaryStateCLK; // Tracks the last state of the rotary encoder CLK
bool rotaryPressed = false; // Tracks if the rotary button was pressed

// Uncomment if using ESP8266 or ESP32
void IRAM_ATTR count() {
  Sensor.count();
}

void setup() {
  Serial.begin(115200);
  Sensor.begin(count);

  pinMode(Relay_Valve, OUTPUT);
  pinMode(Relay_LED, OUTPUT);
  pinMode(Relay_Pompa, OUTPUT);
  pinMode(CLK_Rotary, INPUT);
  pinMode(DT_Rotary, INPUT);
  pinMode(SW_Rotary, INPUT_PULLUP);

  // Ensure everything is initially off
  digitalWrite(Relay_Valve, LOW); // Valve closed
  digitalWrite(Relay_LED, LOW);  // LED off
  digitalWrite(Relay_Pompa, LOW); // Pump off

  lastRotaryStateCLK = digitalRead(CLK_Rotary);
prev_CLK_state = digitalRead(CLK_Rotary);


  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");

  Serial.println("System ready. Use the rotary encoder to navigate and select options.");
}

void loop() {
  handleRotaryInput();

  lcd.setCursor(0, 0);
  lcd.print("Option: ");
  lcd.setCursor(8, 0);
  lcd.print(rotaryPosition);

  switch (rotaryPosition) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print("Start Process   ");
      if (rotaryPressed) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Process Running");
        Serial.println("Process started.");
        runProcess();
      }
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print("Control Pump    ");
      if (rotaryPressed) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Pump Control");
        Serial.println("Pump control started.");
        controlPump();
      }
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print("Control Valve   ");
      if (rotaryPressed) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Valve Control");
        Serial.println("Valve control started.");
        controlValve();
      }
      break;
    case 3:
      lcd.setCursor(0, 1);
      lcd.print("Set Max Volume  ");
      if (rotaryPressed) {
        adjustMaxVolume();
      }
      break;
    case 4:
      lcd.setCursor(0, 1);
      lcd.print("Reset Sensor    ");
      if (rotaryPressed) {
        Sensor.resetVolume();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sensor Reset");
        delay(1000);
        lcd.clear();
        Serial.println("Sensor volume reset.");
      }
      break;
    case 5:
     lcd.setCursor(0, 1);
     lcd.print("Manual Mode     ");
      if (rotaryPressed) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Manual Running");
      Serial.println("Manual process started.");
      runManualProcess();
    }
  break;
    default:
      break;
  }
}

void handleRotaryInput() {
  int currentStateCLK = digitalRead(CLK_Rotary);
  int currentStateSW = digitalRead(SW_Rotary);

  // Detect rotary encoder rotation
  if (currentStateCLK != lastRotaryStateCLK && currentStateCLK == LOW) {
    if (digitalRead(DT_Rotary) != currentStateCLK) {
      rotaryPosition++;
    } else {
      rotaryPosition--;
    }

    if (rotaryPosition < 0) rotaryPosition = 5; // Wrap around menu
    if (rotaryPosition > 5) rotaryPosition = 0;

    Serial.print("Selected option: ");
    Serial.println(rotaryPosition);
  }

  lastRotaryStateCLK = currentStateCLK;

  // Detect rotary button press
  if (currentStateSW == LOW && !rotaryPressed) {
    rotaryPressed = true;
  } else if (currentStateSW == HIGH && rotaryPressed) {
    rotaryPressed = false;
  }
}

void runProcess() {
  digitalWrite(Relay_Valve, HIGH); // Open valve
  digitalWrite(Relay_Pompa, HIGH); // Start pump

  while (true) {
    Sensor.read();
    float volume = Sensor.getVolume();

    lcd.setCursor(0, 1);
    lcd.print("Volume: ");
    lcd.print(volume, 2);
    lcd.print("L   ");

    if (millis() - timebefore >= 1000) {
      Serial.print("Volume (L): ");
      Serial.println(volume);
      timebefore = millis();
    }

    if (volume >= maxVolume) {
      digitalWrite(Relay_Valve, LOW); // Close valve
      digitalWrite(Relay_Pompa, LOW); // Stop pump

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Target Reached");
      delay(2000);

      Serial.println("Target volume reached. Pump and valve stopped.");
      Serial.println("Program completed.");

      for (int i = 0; i < 3; i++) {
        digitalWrite(Relay_LED, HIGH);
        delay(500);
        digitalWrite(Relay_LED, LOW);
        delay(500);
      }
      break;
    }
  }
}

void controlPump() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pump Control");
  lcd.setCursor(0, 1);
  lcd.print("ON/OFF: Rotate");

  while (true) {
    handleRotaryInput();
    if (rotaryPressed) {
      if (rotaryPosition == 0) {
        digitalWrite(Relay_Pompa, HIGH);
        lcd.setCursor(0, 1);
        lcd.print("Pump: ON       ");
        Serial.println("Pump is ON.");
      } else if (rotaryPosition == 1) {
        digitalWrite(Relay_Pompa, LOW);
        lcd.setCursor(0, 1);
        lcd.print("Pump: OFF      ");
        Serial.println("Pump is OFF.");
      } else if (rotaryPosition == 2) {
        Serial.println("Exiting pump control.");
        lcd.clear();
        break;
      }
    }
  }
}

void controlValve() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Valve Control");
  lcd.setCursor(0, 1);
  lcd.print("OPEN/CLOSE: Rot");

  while (true) {
    handleRotaryInput();
    if (rotaryPressed) {
      if (rotaryPosition == 0) {
        digitalWrite(Relay_Valve, HIGH);
        lcd.setCursor(0, 1);
        lcd.print("Valve: OPEN    ");
        Serial.println("Valve is OPEN.");
      } else if (rotaryPosition == 1) {
        digitalWrite(Relay_Valve, LOW);
        lcd.setCursor(0, 1);
        lcd.print("Valve: CLOSED  ");
        Serial.println("Valve is CLOSED.");
      } else if (rotaryPosition == 2) {
        Serial.println("Exiting valve control.");
        lcd.clear();
        break;
      }
    }
  }
}

void adjustMaxVolume() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Max Volume");
  lcd.setCursor(0, 1);
  lcd.print("Rotate to adj.");

  while (true) {
     CLK_state = digitalRead(CLK_Rotary);
 if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
    if (digitalRead(DT_Rotary) == HIGH) {
      maxVolume--;
    } else {
      maxVolume++;    
    }
  }
  prev_CLK_state = CLK_state;

    if (rotaryPressed) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Max Volume: ");
      lcd.print(maxVolume, 1);
      delay(1000);
      break;
    }

    maxVolume += (rotaryPosition > 0) ? 0.1 : -0.1;
    if (maxVolume < 0.1) maxVolume = 0.1; // Prevent negative or zero values
    lcd.setCursor(0, 1);
    lcd.print("Max: ");
    lcd.print(maxVolume, 1);
    lcd.print(" L   ");
    delay(100);
  }
}

// Add this function to handle the manual mode
void runManualProcess() {
  const float targetVolume = 200.0; // Target volume in mL
  const float flowRate = 58.33;    // Flow rate in mL/s (3.5 L/min)
  unsigned long runTime = (targetVolume / flowRate) * 1000; // Calculate run time in ms

  digitalWrite(Relay_Valve, HIGH); // Open valve
  digitalWrite(Relay_Pompa, HIGH); // Start pump
  
  lcd.setCursor(0, 1);
  lcd.print("Dispensing...");
  Serial.println("Manual process running...");

  delay(runTime); // Wait until the target volume is dispensed

  digitalWrite(Relay_Valve, LOW); // Close valve
  digitalWrite(Relay_Pompa, LOW); // Stop pump

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Manual Complete");
  delay(2000);

  Serial.println("Manual process completed. Pump and valve stopped.");
}