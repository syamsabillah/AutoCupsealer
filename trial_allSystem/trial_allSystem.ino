#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <ezButton.h>
#include <EEPROM.h> // Include EEPROM library

#define SDA_LCD 21
#define SCL_LCD 22
#define CLK_Rotary 16
#define DT_Rotary 17
#define SW_Rotary 5
#define Button_enter 34
#define Button_reset 39
#define Button_manual 35

//LCD
LiquidCrystal_I2C lcd(0x27,16,2);

//rotary encoder
#define DIRECTION_CW  0   // clockwise direction
#define DIRECTION_CCW 1  // counter-clockwise direction

int counter_air = 0;
int counter_botol = 0;
int counter_manual = 0;
int direction = DIRECTION_CW;
int CLK_state;
int prev_CLK_state;
ezButton button(SW_Rotary);

//button
ezButton enter(Button_enter);  // create ezButton object for button 1
ezButton reset(Button_reset);  // create ezButton object for button 2
ezButton manual(Button_manual);  // create ezButton object for button 3

//System
int System_ready;

void setup() {
  Serial.begin(9600); // Start serial communication

  // Set button pins to INPUT_PULLDOWN
  pinMode(Button_enter, INPUT_PULLDOWN);
  pinMode(Button_reset, INPUT_PULLDOWN);
  pinMode(Button_manual, INPUT_PULLDOWN);

  // Configure buttons with inverted logic
  enter.setDebounceTime(100);
  reset.setDebounceTime(100);
  manual.setDebounceTime(100);

  // Encoder
  pinMode(CLK_Rotary, INPUT);
  pinMode(DT_Rotary, INPUT);
  button.setDebounceTime(50);
  prev_CLK_state = digitalRead(CLK_Rotary);

  lcd.init();
  lcd.backlight();

  // Load saved values from EEPROM
  counter_air = EEPROM.read(0);       // Address 0 for counter_air
  counter_botol = EEPROM.read(1);     // Address 1 for counter_botol

  System_ready = 0;
}

void loop() {
  enter.loop();  // MUST call the loop() function first
  reset.loop();  // MUST call the loop() function first
  manual.loop(); // MUST call the loop() function first

  switch (System_ready) {
    case 0:
      startUp();
      break;
    case 1:
      setting_takaran_air();
      break;
    case 2:
      setting_counting();
      break;
    case 3:
      running_system();
      break;
    case 4:
      sistem_manual();
      break;
    default:
      System_ready = 0; // Reset to initial state
      break;
  }
}

void running_system() {
  static int last_counter_botol = -1;
  if (counter_botol != last_counter_botol) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Running System");
    lcd.setCursor(0, 1);
    lcd.print("Counting "); lcd.print(counter_botol);
    last_counter_botol = counter_botol;
  }
  Serial.println("running System");
  if (manual.isPressed()) {
    System_ready = 4;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pindah plate");
    lcd.setCursor(0, 1);
    lcd.print("Putar Knob");
  }
}

void sistem_manual() {
  button.loop();
  CLK_state = digitalRead(CLK_Rotary);
  if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
    if (digitalRead(DT_Rotary) == HIGH) {
      counter_manual--;
      direction = DIRECTION_CCW;
    } else {
      counter_manual++;
      direction = DIRECTION_CW;
    }
  }
  prev_CLK_state = CLK_state;

  static int last_counter_manual = -1;
  if (counter_manual != last_counter_manual) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pindah Manual");
    lcd.setCursor(0, 1);
    lcd.print(counter_manual);
    Serial.print("Pindah Manual: ");
    Serial.println(counter_manual);
    last_counter_manual = counter_manual;
  }

  if (enter.isPressed()) {
    System_ready = 1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Takaran Saji");
    lcd.setCursor(0, 1);
    lcd.print("Putar Knob");
  }
  if (reset.isPressed()) {
    counter_manual = 0;
  }
}

void startUp() {
  static bool lcdUpdated = false;

  if (!lcdUpdated) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Autocup Sealer");
    lcd.setCursor(0, 1);
    lcd.print("Tekan Enter");
    Serial.println("Tekan Enter");
    lcdUpdated = true;
  }

  if (enter.isPressed()) {
    lcdUpdated = false;
    System_ready = 1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Takaran Saji");
    lcd.setCursor(0, 1);
    lcd.print("Putar Knob");
  }
}

void setting_takaran_air() {
  button.loop();
  CLK_state = digitalRead(CLK_Rotary);

  if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
    if (digitalRead(DT_Rotary) == HIGH) {
      if (counter_air > 0) {
        counter_air--;
        direction = DIRECTION_CCW;
      }
    } else {
      counter_air++;
      direction = DIRECTION_CW;
    }
  }
  prev_CLK_state = CLK_state;

  static int last_counter_air = -1;
  if (counter_air != last_counter_air) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Takaran Saji");
    lcd.setCursor(0, 1);
    lcd.print(counter_air);
    lcd.setCursor(13, 1);
    lcd.print("ml");
    Serial.print("Takaran Saji: ");
    Serial.println(counter_air);
    last_counter_air = counter_air;
  }
  if (enter.isPressed()) {
    EEPROM.write(0, counter_air);
    System_ready = 2;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Jumlah");
    lcd.setCursor(0, 1);
    lcd.print(counter_botol);
  }
  if (reset.isPressed()) {
    counter_air = 0;
  }
}

void setting_counting() {
  static int last_counter_botol = -1;
  if (counter_botol != last_counter_botol) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Jumlah");
    lcd.setCursor(0, 1);
    lcd.print(counter_botol);
    Serial.print("Jumlah: ");
    Serial.println(counter_botol);
    last_counter_botol = counter_botol;
  }

  if (enter.isPressed()) {
    EEPROM.write(1, counter_botol);
    System_ready = 3;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Berjalan");
    lcd.setCursor(0, 1);
    lcd.print("Jumlah "); lcd.print(counter_botol);
  }
  if (reset.isPressed()) {
    counter_botol = 0;
  }
} 
