#include <LiquidCrystal_I2C.h>

#define 1A 50

#define CURRENT_PIN 18
#define MOSFET_PIN 3
#define START_BTN_INT 0

LiquidCrystal_I2C lcd(0x20,16,2);

bool run = 0;
long start_time = 0;

void setup() {
  pinMode(MOSFET_PIN, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("TEST");

  attachInterrupt(0, _startInterrupt, FALLING);
}

void loop() {
  if (run) {
    
  }
}

void _statrInterrupt() {
  start_time = millis();
  run = 1;
}
