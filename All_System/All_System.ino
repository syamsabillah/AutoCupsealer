#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <ezButton.h>
#include <EEPROM.h> // Include EEPROM library
#include <FlowSensor.h>


// Pin Definition
#define Relay_Valve 15
#define Relay_LED 2
#define Relay_Pompa 23
#define Flow 13
#define type OF05ZAT
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
#define CLK_Rotary 16
#define DT_Rotary 17
#define Button_enter 34
#define Button_reset 36
#define Button_manual 35
#define motorPin1 0
#define motorPin2 4
#define enablePin 39

//flow
FlowSensor Sensor(type, Flow);
unsigned long timebefore = 0; // same type as millis()

//LCD
LiquidCrystal_I2C lcd(0x27,16,2);

//Accell
AccelStepper stepper1(AccelStepper::FULL2WIRE, Pul_1, Dir_1);
AccelStepper stepper2(AccelStepper::FULL2WIRE, Pul_2, Dir_2);
AccelStepper stepper3(AccelStepper::FULL2WIRE, Pul_3, Dir_3);
AccelStepper stepper4(AccelStepper::FULL2WIRE, Pul_4, Dir_4);

//rotary encoder
#define DIRECTION_CW  0   // clockwise direction
#define DIRECTION_CCW 1  // counter-clockwise direction

int counter_air = 0;
int counter_botol = 0;
int counter_manual = 0;
int direction = DIRECTION_CW;
int CLK_state;
int prev_CLK_state;

//button
ezButton enter(Button_enter, INPUT_PULLDOWN);  // create ezButton object for button 1
ezButton reset(Button_reset, INPUT_PULLDOWN);  // create ezButton object for button 2
ezButton manual(Button_manual, INPUT_PULLDOWN);  // create ezButton object for button 3
ezButton limit_switch1(Limit_Switch, INPUT_PULLDOWN);  // create ezButton object for button 4
ezButton limit_switch2(Limit_Switch2, INPUT_PULLDOWN);  // create ezButton object for button 4

//System
int System_ready;
int drop_cup,roll_plastik;

void IRAM_ATTR count()
{
  Sensor.count();
}

void setup() {
  Serial.begin(9600); // Start serial communication

  //flow
  Sensor.begin(count);

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
  pinMode(Button_enter, INPUT_PULLDOWN);
  pinMode(Button_reset, INPUT_PULLDOWN);
  pinMode(Button_manual, INPUT_PULLDOWN);
  pinMode(Limit_Switch, INPUT_PULLDOWN);
  pinMode(Limit_Switch2, INPUT_PULLDOWN);

  pinMode(Relay_Valve, OUTPUT);
  pinMode(Relay_LED, OUTPUT);
  pinMode(Relay_Pompa, OUTPUT);

  // Ensure everything is initially off
  digitalWrite(Relay_Valve, LOW); // Valve closed
  digitalWrite(Relay_LED, LOW);  // LED off
  digitalWrite(Relay_Pompa, LOW); // Pump off
  
  //set button
  enter.setDebounceTime(100);  // set debounce time to 100 milliseconds
  reset.setDebounceTime(100);  // set debounce time to 100 milliseconds
  manual.setDebounceTime(100);  // set debounce time to 100 milliseconds
  limit_switch1.setDebounceTime(100);  // set debounce time to 100 milliseconds
  limit_switch2.setDebounceTime(100);  // set debounce time to 100 milliseconds


  //encoder
 // configure encoder pins as inputs
  pinMode(CLK_Rotary, INPUT);
  pinMode(DT_Rotary, INPUT);
  
  // read the initial state of the rotary encoder's CLK pin
  prev_CLK_state = digitalRead(CLK_Rotary);

  //first step run lcd
  lcd.init();
  lcd.backlight();
  
  // Load saved values from EEPROM
  counter_air = EEPROM.read(0);       // Address 0 for counter_air
  counter_botol = EEPROM.read(1);     // Address 1 for counter_botol


  System_ready=0;
}

void loop() {
  enter.loop();  // MUST call the loop() function first
  reset.loop();  // MUST call the loop() function first
  manual.loop();  // MUST call the loop() function first
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
      setting_counting();
      break;
    case 3 :
      running_system();
      break;
    case 4 :
      sistem_manual();
      break;
    default:
      System_ready = 0; // Reset to initial state
      break;
    
  }

}

void running_system(){
   digitalWrite(Relay_LED, HIGH); // LED on
  static int last_counter_botol = -1;
  if (counter_botol != last_counter_botol) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Running System");
    lcd.setCursor(0, 1);
    lcd.print("Counting "); lcd.print(counter_botol);
    last_counter_botol = counter_botol;
  }
  drop_cup+=6500; 
  roll_plastik-= 6500; 

  // Move stepper motors
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

  stepper1.moveTo(-17000); // Go to filling station
  while (stepper1.distanceToGo() != 0) {
    Serial.println("Move to filling");
    stepper1.run();
  }
  Serial.println("filling proses");
  // Filling logic here
  filling();
  //delay(1000);
  stepper1.moveTo(-36000); // Go to press station
  while (stepper1.distanceToGo() != 0) {
    Serial.println("Move to press");
    stepper1.run(); 
  }

  stepper3.moveTo(roll_plastik);
  while (stepper3.distanceToGo() != 0) {
    Serial.println("roll plastik");
    stepper3.run();
  }

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

  counter_botol++;
}

void sistem_manual(){

  stepper1.stop();
   // Move stepper motors
  stepper1.moveTo(-300); // Move to drop cup station
  while (stepper1.distanceToGo() != 0) {
    Serial.println("Move to exit");
    stepper1.run();
  }
  stepper4.moveTo(0); // Move to drop cup station
  while (stepper4.distanceToGo() != 0) {
    Serial.println("turun");
    stepper4.run();
  }


  if(enter.isPressed()){
    System_ready=1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Takaran air");
    lcd.setCursor(0, 1);
    lcd.print("Putar Knob");
  }
  if(reset.isPressed()){
    counter_manual = 0;
  }
}

void startUp() {
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

  if (enter.isPressed()) {
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
        counter_air-10;
        direction = DIRECTION_CCW;
      }
    } else {
      counter_air+10;
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
  if(enter.isPressed()){
    EEPROM.write(0, counter_air); // Save counter_air to EEPROM
    System_ready=2;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Counting");
    lcd.setCursor(0, 1);
    lcd.print(counter_botol);
    Serial.print("Counting: ");
    Serial.println(counter_botol);
    
  }
  if(reset.isPressed()){
    counter_air = 0;
  }
}

void setting_counting(){
//first step run lcd
  static int last_counter_botol = -1;
  if (counter_botol != last_counter_botol) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Counting");
    lcd.setCursor(0, 1);
    lcd.print(counter_botol);
    Serial.print("Counting: ");
    Serial.println(counter_botol);
    last_counter_botol = counter_botol;
  }
  

  if(enter.isPressed()){
    EEPROM.write(1, counter_botol); // Save counter_botol to EEPROM
    System_ready=3;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Running System");
    lcd.setCursor(0, 1);
    lcd.print("Counting ");lcd.print(counter_botol);
  }
  if(reset.isPressed()){
    counter_botol = 0;
  }

}

void filling() { 
  const float flowRate = 58.33;    // Flow rate in mL/s (3.5 L/min)
  unsigned long runTime = (counter_air / flowRate) * 1000; // Calculate run time in ms

  digitalWrite(Relay_Valve, HIGH); // Open valve
  digitalWrite(Relay_Pompa, HIGH); // Start pump
  
  lcd.setCursor(0, 1);
  lcd.print("Dispensing...");
  Serial.println("Manual process running...");

  delay(1000); // Wait until the target volume is dispensed

  digitalWrite(Relay_Valve, LOW); // Close valve
  digitalWrite(Relay_Pompa, LOW); // Stop pump

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Manual Complete");
}


