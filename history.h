#include <Arduino.h>
#include <GyverButton.h>
#include <LiquidCrystal_I2C.h>
#include <avr/eeprom.h>

#define OFFSET 100
#define COUNT 0
#define DATA 1

struct Result {
  float capacity;
  long time;  
};

uint8_t cursor = 0;
uint8_t count = 0;

void draw(LiquidCrystal_I2C& lcd, Result& result) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Capaciry: ");
  lcd.print(result.capacity);
  lcd.print("mAh");

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(result.time / 3600000);
  lcd.print("h");

  lcd.setCursor(15, 1);
  lcd.print(cursor);
}

void prepare_history_eeprom() {
  eeprom_update_byte(OFFSET + COUNT, 0);
}

void write_history(Result& result) {
  count = eeprom_read_byte(OFFSET + COUNT);
  if (count >= 9) count = 0;

  eeprom_update_block((void*) &result, OFFSET + DATA + ((count + 1) * sizeof(result)), sizeof(result));
  eeprom_update_byte(OFFSET + COUNT, count + 1);
}

void inn(LiquidCrystal_I2C& lcd, GButton& button) {
  count = eeprom_read_byte(OFFSET + COUNT);
  if (count == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("history is clean");
    lcd.setCursor(0, 1);
    lcd.print("Press to exit");
  } else {
    Result r;
    eeprom_read_block((void*) &r, OFFSET + DATA + cursor * sizeof(r), sizeof(r));
    draw(lcd, r);
  }

  for (;;) {
    button.tick();

    if (button.isSingle()) {
      if (count == 0) break;
      else {
        cursor++;
        if (cursor > count) cursor = 0;
        Result r;
        eeprom_read_block((void*) &r, OFFSET + DATA + cursor * sizeof(r), sizeof(r));
        draw(lcd, r);
      }
    }
    if (button.isTriple()) {
      break;
    }
  }
}
