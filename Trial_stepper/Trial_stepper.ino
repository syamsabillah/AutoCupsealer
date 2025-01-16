#include <AccelStepper.h>

#define Pul_1 27
#define Dir_1 14

AccelStepper stepper3(AccelStepper::FULL2WIRE, Pul_1, Dir_1);

void setup() {
  Serial.begin(9600); // Start serial communication
  while (!Serial) {
    ; // Wait for the serial port to connect
  }

  Serial.println("Enter speed, acceleration, and target position separated by commas:");
  stepper3.setMaxSpeed(1000); // Default speed
  stepper3.setAcceleration(500); // Default acceleration
}

void loop() {
  static float speed = 1000;         // Default speed
  static float acceleration = 500;  // Default acceleration
  static long targetPosition = 8000; // Default target position

  // Check if data is available on the serial port
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Read input until newline
    input.trim(); // Remove any extra whitespace

    // Parse the input
    int firstComma = input.indexOf(',');
    int secondComma = input.indexOf(',', firstComma + 1);

    if (firstComma > 0 && secondComma > firstComma) {
      // Extract values from the input
      speed = input.substring(0, firstComma).toFloat();
      acceleration = input.substring(firstComma + 1, secondComma).toFloat();
      targetPosition = input.substring(secondComma + 1).toInt();

      // Update stepper settings
      stepper3.setMaxSpeed(speed);
      stepper3.setAcceleration(acceleration);

      Serial.println("Updated values:");
      Serial.print("Speed: ");
      Serial.println(speed);
      Serial.print("Acceleration: ");
      Serial.println(acceleration);
      Serial.print("Target Position: ");
      Serial.println(targetPosition);
    } else {
      Serial.println("Invalid input. Use format: speed,acceleration,targetPosition");
    }
  }

  // Set the target position and run to it
  stepper3.moveTo(targetPosition);
  stepper3.runToPosition();

  delay(1000);

  // Move back to zero
  stepper3.moveTo(0);
  stepper3.runToPosition();

  delay(1000);
}
