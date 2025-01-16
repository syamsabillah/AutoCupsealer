#include <Wire.h> 
#include <ezButton.h>


#define Limit_Switch 0
#define Limit_Switch2 12

ezButton limit_switch1(Limit_Switch );  // create ezButton object for button 4
ezButton limit_switch2(Limit_Switch2);  // create ezButton object for button 4

void setup() {
  Serial.begin(115200);

  limit_switch1.setDebounceTime(50);  // set debounce time to 100 milliseconds
  limit_switch2.setDebounceTime(50);  // set debounce time to 100 milliseconds
}

void loop() {
  limit_switch1.loop();  // MUST call the loop() function first
  limit_switch2.loop();  // MUST call the loop() function first
  int lmt1 = limit_switch1.getState();
  int lmt2 = limit_switch2.getState();


  
  if (limit_switch1.isPressed())
    {
    Serial.println("limit 1 is pressed");
    Serial.println(lmt1);
    }
  
  if (limit_switch2.isPressed())
    {
    Serial.println("limit 2 is pressed");
    Serial.println(lmt2);
    }
  
}
