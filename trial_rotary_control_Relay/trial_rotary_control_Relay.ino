#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ezButton.h>

#define SDA_LCD 21
#define SCL_LCD 22
#define CLK_Rotary 16
#define DT_Rotary 17
#define SW_Rotary 5

LiquidCrystal_I2C lcd(0x27, 16, 2);
ezButton rotaryButton(SW_Rotary);

int CLK_state;
int prev_CLK_state;

bool inSystem = false;
bool isFilling = false;
int takaranAir = 0;

void setup() {
  pinMode(CLK_Rotary, INPUT);
  pinMode(DT_Rotary, INPUT);
  rotaryButton.setDebounceTime(50);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Tekan enter");
  lcd.setCursor(0, 1);
  lcd.print("untuk masuk sistem");

  prev_CLK_state = digitalRead(CLK_Rotary);
}

void loop() {
  rotaryButton.loop();

  // Reset system if button is held for 2 seconds
  if (rotaryButton.isPressed() && isFilling) {
    unsigned long pressStart = millis();
    while (rotaryButton.isPressed()) {
      if (millis() - pressStart > 2000) {
        resetSystem();
        return;
      }
    }
  }

  // Handle button press to enter the system or confirm value
  if (rotaryButton.isPressed()) {
    if (!inSystem) {
      inSystem = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Takaran air:");
      lcd.setCursor(0, 1);
      lcd.print(takaranAir);
    } else if (!isFilling) {
      // Start the filling process
      isFilling = true;
      simulateFilling();
    }
  }

  // Handle rotary encoder rotation to set "takaran air"
  if (inSystem && !isFilling) {
    CLK_state = digitalRead(CLK_Rotary);
    if (CLK_state != prev_CLK_state) {
      if (digitalRead(DT_Rotary) != CLK_state) {
        takaranAir++; // Increase value
      } else {
        takaranAir = max(0, takaranAir - 1); // Decrease value, prevent negative
      }
      prev_CLK_state = CLK_state;

      // Update LCD with the new value
      lcd.setCursor(0, 1);
      lcd.print("      "); // Clear previous value
      lcd.setCursor(0, 1);
      lcd.print(takaranAir);
    }
  }
}

void simulateFilling() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistem mengisi");

  for (int currentValue = 0; currentValue <= takaranAir; currentValue++) {
    lcd.setCursor(0, 1);
    lcd.print("      "); // Clear previous value
    lcd.setCursor(0, 1);
    lcd.print(currentValue);
    delay(1000); // Simulate 1-second delay per step

    // Check if the button is held during the filling process
    rotaryButton.loop();
    if (rotaryButton.isPressed()) {
      unsigned long pressStart = millis();
      while (rotaryButton.isPressed()) {
        if (millis() - pressStart > 2000) {
          resetSystem();
          return;
        }
      }
    }
  }

  delay(1000); // Brief pause after filling
  resetSystem(); // Automatically reset after filling
}

void resetSystem() {
  inSystem = false;
  isFilling = false;
  takaranAir = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tekan enter");
  lcd.setCursor(0, 1);
  lcd.print("untuk masuk sistem");
}
