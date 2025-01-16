#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <ezButton.h>

#define Pul_1 27
#define Pul_2 25
#define Pul_3 32
#define Pul_4 19
#define Dir_1 14
#define Dir_2 26
#define Dir_3 33
#define Dir_4 18
#define SDA_LCD 21
#define SCL_LCD 22
#define Button_reset 39

AccelStepper stepper1(AccelStepper::FULL2WIRE, Pul_1, Dir_1);
AccelStepper stepper2(AccelStepper::FULL2WIRE, Pul_2, Dir_2);
AccelStepper stepper3(AccelStepper::FULL2WIRE, Pul_3, Dir_3);
AccelStepper stepper4(AccelStepper::FULL2WIRE, Pul_4, Dir_4);

LiquidCrystal_I2C lcd(0x27, 16, 2);
ezButton reset(Button_reset);

int motor_selected = 1; // Default motor to control
float speed = 100.0;    // Default speed
long distance = 0;      // Distance input via serial
AccelStepper* selectedStepper = &stepper1; // Pointer to the selected stepper

void setup() {
  Serial.begin(9600);

  pinMode(Button_reset, INPUT_PULLDOWN);
  reset.setDebounceTime(100);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Select Motor:");

  // Initialize steppers
  stepper1.setMaxSpeed(10000);
  stepper1.setAcceleration(10000);
  stepper2.setMaxSpeed(10000);
  stepper2.setAcceleration(10000);
  stepper3.setMaxSpeed(10000);
  stepper3.setAcceleration(10000);
  stepper4.setMaxSpeed(10000);
  stepper4.setAcceleration(10000);
}

void loop() {
  reset.loop();

  // Check for serial input to select motor, speed, or position
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("M")) {
      motor_selected = input.substring(1).toInt();
      switch (motor_selected) {
        case 1:
          selectedStepper = &stepper1;
          break;
        case 2:
          selectedStepper = &stepper2;
          break;
        case 3:
          selectedStepper = &stepper3;
          break;
        case 4:
          selectedStepper = &stepper4;
          break;
        default:
          Serial.println("Invalid motor selection.");
          return;
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Motor: ");
      lcd.print(motor_selected);
      Serial.println("Motor selected: " + String(motor_selected));
    } else if (input.startsWith("S")) {
      speed = input.substring(1).toFloat();
      selectedStepper->setMaxSpeed(speed);
      Serial.println("Speed set to: " + String(speed));
    } else if (input.startsWith("P")) {
      distance = input.substring(1).toInt();
      Serial.println("Position set to: " + String(distance));

      // Run the motor to the specified position
      selectedStepper->moveTo(distance);
      Serial.println("Running motor...");
      while (selectedStepper->distanceToGo() != 0) {
        selectedStepper->run();
      }
      Serial.println("Motor stopped.");
    } else {
      Serial.println("Invalid input. Use M<number> for motor, S<number> for speed, P<number> for position.");
    }
  }

  // Check reset button
  if (reset.isPressed()) {
    selectedStepper->stop();
    selectedStepper->runToPosition();
    Serial.println("Emergency stop activated.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Emergency Stop!");
  }

  // Move the motor to the specified distance
  if (selectedStepper->distanceToGo() != 0) {
    selectedStepper->run();
  }
}
