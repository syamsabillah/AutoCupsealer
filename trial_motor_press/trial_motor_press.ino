#include "BTS7960.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const uint8_t EN = 12;
const uint8_t L_PWM = 4;
const uint8_t R_PWM = 0;

BTS7960 motorController(EN, L_PWM, R_PWM);

#define SDA_LCD 21
#define SCL_LCD 22
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Motor supra");

  // Initialize serial communication
  Serial.begin(9600);
  lcd.setCursor(0, 1);
  lcd.print("Awaiting cmd...");

  // Enable motor controller
  motorController.Enable();
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      handleCommand(input);
    }
  }
}

void handleCommand(String command) {
  // Parse the command format: "DIR SPEED DELAY"
  // DIR: LEFT or RIGHT
  // SPEED: 0-255
  // DELAY: time in milliseconds

  int firstSpace = command.indexOf(' ');
  int secondSpace = command.indexOf(' ', firstSpace + 1);

  if (firstSpace == -1 || secondSpace == -1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid cmd");
    return;
  }

  String dir = command.substring(0, firstSpace);
  int speed = command.substring(firstSpace + 1, secondSpace).toInt();
  int delayTime = command.substring(secondSpace + 1).toInt();

  if (speed < 0 || speed > 255 || delayTime < 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid vals");
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dir: ");
  lcd.print(dir);
  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print(speed);

  motorController.Enable();

  if (dir.equalsIgnoreCase("LEFT")) {
    motorController.Enable();
    motorController.TurnLeft(speed);
  } else if (dir.equalsIgnoreCase("RIGHT")) {
    motorController.TurnRight(speed);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid dir");
    return;
  }

  delay(delayTime);
  motorController.Stop();
  motorController.Disable();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Motor stopped");
}
