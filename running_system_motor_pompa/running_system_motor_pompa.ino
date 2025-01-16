#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <ezButton.h>

// Pin Definition
#define Relay_Valve 15
#define Relay_LED 4
#define Relay_Pompa 23
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
#define Limit_Switch 1
#define CLK_Rotary 16
#define DT_Rotary 17
#define SW_Rotary 5
#define Limit_switch 35
#define Limit_Switch 0
#define Limit_Switch2 12

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//rotary encoder
#define DIRECTION_CW  0   // clockwise direction
#define DIRECTION_CCW 1  // counter-clockwise direction
int direction = DIRECTION_CW;

ezButton limit_switch1(Limit_Switch );  // create ezButton object for button 4
ezButton limit_switch2(Limit_Switch2);  // create ezButton object for button 4
ezButton button(SW_Rotary);
int CLK_state;
int prev_CLK_state;

// AccelStepper
AccelStepper stepper1(AccelStepper::FULL2WIRE, Pul_1, Dir_1);
AccelStepper stepper2(AccelStepper::FULL2WIRE, Pul_2, Dir_2);
AccelStepper stepper3(AccelStepper::FULL2WIRE, Pul_3, Dir_3);
AccelStepper stepper4(AccelStepper::FULL2WIRE, Pul_4, Dir_4);

int counter_air = 0;
int counter_botol = 0;

// System
int System_ready;



int drop_cup, roll_plastik;
void setup() {
  Serial.begin(115200); // Start serial communication
  Serial.print("Sistem ready");

  pinMode(CLK_Rotary, INPUT);
  pinMode(DT_Rotary, INPUT);
  button.setDebounceTime(50);
  limit_switch1.setDebounceTime(50);  // set debounce time to 100 milliseconds
  limit_switch2.setDebounceTime(50);  // set debounce time to 100 milliseconds

  // Initialize steppers
  stepper1.setMaxSpeed(18000);
  stepper1.setAcceleration(15000);
  stepper2.setMaxSpeed(10000);
  stepper2.setAcceleration(10000);
  stepper3.setMaxSpeed(10000);
  stepper3.setAcceleration(10000);
  stepper4.setMaxSpeed(18000);
  stepper4.setAcceleration(15000);

  pinMode(Relay_Valve, OUTPUT);
  pinMode(Relay_LED, OUTPUT);
  pinMode(Relay_Pompa, OUTPUT);

  // Ensure everything is initially off
  digitalWrite(Relay_Valve, LOW); // Valve closed
  digitalWrite(Relay_LED, LOW);  // LED off
  digitalWrite(Relay_Pompa, LOW); // Pump off

  // Initialize LCD
  lcd.init();
  lcd.backlight();  
  counter_air = 100;
  counter_botol = 0;
}

void loop() {
   
  button.loop();  // MUST call the loop() function first
  limit_switch1.loop();  // MUST call the loop() function first
  limit_switch2.loop();  // MUST call the loop() function first
  
  
  switch(System_ready){
    case 0 :
      startUp();
      break;
    case 1 :
      setting_takaran_air();
      break;
    case 2 :   
      executeSystem();  
      break;
    default:
      System_ready = 0; // Reset to initial state
      break;
    
  } 
  
}

void startUp() {
 int limit1 = limit_switch1.getState();
int limit2 = limit_switch2.getState();

if (limit1 == HIGH) {
  stepper1.moveTo(40000); // Move to drop cup station
  while (stepper1.distanceToGo() != 0) {
    stepper1.run();

    // Update the state of limit1 inside the loop
    limit1 = limit_switch1.getState();
    if (limit1 == LOW) {
      stepper1.stop();
      stepper1.setCurrentPosition(0);
      break;
    }
  }
}


    
  
  static bool lcdUpdated = false; // Flag to check if the LCD has been updated

  if (!lcdUpdated) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Autocup Sealer");
    lcd.setCursor(0, 1);
    lcd.print("Tekan Enter");
    Serial.println("tekan enter");
    lcdUpdated = true; // Set the flag to true after updating the LCD
  }

  if (button.isPressed()) {
    lcdUpdated = false; // Reset the flag when transitioning to the next state
    System_ready = 1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Takaran air");
    lcd.setCursor(0, 1);
    lcd.print("Putar Knob");
  }
}

void setting_takaran_air(){ 
  CLK_state = digitalRead(CLK_Rotary);
  
  if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
    if (digitalRead(DT_Rotary) == HIGH) {
      if (counter_air > 0) { // Prevent decrementing below 0
        counter_air-=10;
        direction = DIRECTION_CCW;
      }
    } else {
      counter_air+=10;
      direction = DIRECTION_CW;
    }
  }
  prev_CLK_state = CLK_state;

//first step run lcd
  static int last_counter_air = -1;
  if (counter_air != last_counter_air) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Takaran air");
    lcd.setCursor(0, 1);
    lcd.print(counter_air);
    lcd.print("ml");
    Serial.print("Takaran air: ");
    Serial.println(counter_air);
    last_counter_air = counter_air;
  }
  if(button.isPressed()){
    System_ready=2;    
  }  
}


void executeSystem() {
  Serial.println("System Running...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Running System");
  digitalWrite(Relay_LED, HIGH); // LED on

  static int last_counter_botol = -1;
  if (counter_botol != last_counter_botol) {
    lcd.setCursor(0, 1);
    lcd.print("Counting ");
    lcd.print(counter_botol);
    last_counter_botol = counter_botol;
  }

  drop_cup += 6500;
  roll_plastik -= 6250;

  // Move stepper motors
  stepper1.moveTo(-10000); // Move to drop cup station
  while (stepper1.distanceToGo() != 0) {
    stepper1.run();
  }

  stepper2.moveTo(drop_cup); // Drop cup
  while (stepper2.distanceToGo() != 0) {
    stepper2.run();
  }

  stepper1.moveTo(-17000); // Go to filling station
  while (stepper1.distanceToGo() != 0) {
    stepper1.run();
  }

  // Filling logic here
  filling();

  stepper1.moveTo(-36000); // Go to press station
  while (stepper1.distanceToGo() != 0) {
    stepper1.run();
  }

  stepper3.moveTo(roll_plastik);
  while (stepper3.distanceToGo() != 0) {
    stepper3.run();
  }

  stepper1.moveTo(-500); // Go to exit
  while (stepper1.distanceToGo() != 0) {
    stepper1.run();
  }

  stepper4.moveTo(7000); // Go to exit
  while (stepper4.distanceToGo() != 0) {
    stepper4.run();
  }

  stepper4.moveTo(0); // Go to exit
  while (stepper4.distanceToGo() != 0) {
    stepper4.run();
  }


  counter_botol++;

  Serial.println("Cycle Complete");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cycle Complete");
  lcd.setCursor(0, 1);
  lcd.print("Total: ");
  lcd.print(counter_botol);

  if(button.isPressed()){
    System_ready=0;    
  } 
}

void filling() {
  const float targetVolume = 100.0; // Target volume in mL
  const float flowRate = 58.33;    // Flow rate in mL/s (3.5 L/min)
  unsigned long runTime = (counter_air / flowRate) * 1000; // Calculate run time in ms

  digitalWrite(Relay_Valve, HIGH); // Open valve
  digitalWrite(Relay_Pompa, HIGH); // Start pump

  lcd.setCursor(0, 1);
  lcd.print("Dispensing...");
  Serial.println("Filling process running...");

  delay(runTime); // Wait until the target volume is dispensed

  digitalWrite(Relay_Valve, LOW); // Close valve
  digitalWrite(Relay_Pompa, LOW); // Stop pump

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Berhasil isi");  

   
}
