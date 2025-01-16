#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <ezButton.h>

#define Pul_1 27
#define Pul_2 25
#define Pul_3 32
#define Pul_4 34
#define Dir_1 14
#define Dir_2 26
#define Dir_3 33
#define Dir_4 35
#define SDA_LCD 21
#define SCL_LCD 22
#define Limit_Switch 5
#define Limit_Switch2 39

//LCD
LiquidCrystal_I2C lcd(0x27,16,2);

//Accell
AccelStepper stepper1(AccelStepper::FULL2WIRE, Pul_1, Dir_1);
AccelStepper stepper2(AccelStepper::FULL2WIRE, Pul_2, Dir_2);
AccelStepper stepper3(AccelStepper::FULL2WIRE, Pul_3, Dir_3);
AccelStepper stepper4(AccelStepper::FULL2WIRE, Pul_4, Dir_4);

//button
ezButton limit_switch1(Limit_Switch, INPUT_PULLDOWN);  // create ezButton object for button 4
ezButton limit_switch2(Limit_Switch2, INPUT_PULLDOWN);  // create ezButton object for button 4

bool isHomed = false; // Flag to check if homing is complete
int drop_cup;

void setup() {
   Serial.begin(9600); // Start serial communication

   // Initialize steppers
  stepper1.setMaxSpeed(10000);
  stepper1.setAcceleration(10000);
  stepper2.setMaxSpeed(10000);
  stepper2.setAcceleration(10000);
  stepper3.setMaxSpeed(10000);
  stepper3.setAcceleration(10000);
  stepper4.setMaxSpeed(10000);
  stepper4.setAcceleration(10000);

  // Set button pins to INPUT_PULLDOWN
  pinMode(Limit_Switch, INPUT_PULLDOWN);
  pinMode(Limit_Switch2, INPUT_PULLDOWN);

  //set button
  limit_switch1.setDebounceTime(100);  // set debounce time to 100 milliseconds
  limit_switch2.setDebounceTime(100);  // set debounce time to 100 milliseconds

  //first step run lcd
  lcd.init();
  lcd.backlight();

  

}

void loop() {
  limit_switch1.loop(); // Update limit switch states
  limit_switch2.loop();
  int lmtswtch = limit_switch2.getState();

  // Homing sequence
  if (!isHomed) {
    if (lmtswtch != 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("go home");
      stepper1.moveTo(30000); // Move to drop cup station
        while (stepper1.distanceToGo() != 0) {
          Serial.println("Move to drop Cup");
          stepper1.run();
       }
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Stop");
      stepper1.stop();
      stepper1.setCurrentPosition(0); // Set zero position
      isHomed = true;                // Mark homing as complete
      return;                        // End this iteration
    }
  }
  drop_cup+=6500;  

  // Move stepper motors
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("drop station");
  stepper1.moveTo(-10000); // Move to drop cup station
  while (stepper1.distanceToGo() != 0) {
    Serial.println("Move to drop Cup");
    stepper1.run();
  }
  
  stepper2.moveTo(drop_cup); // Drop cup
  while (stepper2.distanceToGo() != 0) {
    Serial.println("cup turun");
    stepper2.run();
  }
  lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("filling");
  stepper1.moveTo(-17000); // Go to filling station
  while (stepper1.distanceToGo() != 0) {
    Serial.println("Move to filling");
    stepper1.run();
  }
  Serial.println("filling proses");
  // Filling logic here
  lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("filling proses");
  delay(1000);
  stepper1.moveTo(-36000); // Go to press station
  while (stepper1.distanceToGo() != 0) {
    Serial.println("Move to press");
    stepper1.run();
  }
lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("roll plastik");
  stepper3.moveTo(5000);
  while (stepper3.distanceToGo() != 0) {
    Serial.println("roll plastik");
    stepper3.run();
  }
lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("exit");
  stepper1.moveTo(-500); // Go to exit
  while (stepper1.distanceToGo() != 0) {
    Serial.println("Move to exit");
    stepper1.run();
  }

  stepper4.moveTo(7000); // Go to exit
  while (stepper4.distanceToGo() != 0) {
    Serial.println("angkat buang");
    stepper4.run();
  }

  stepper4.moveTo(0); // Go to exit
  while (stepper4.distanceToGo() != 0) {
    Serial.println("turun");
    stepper4.run();
  }


}
