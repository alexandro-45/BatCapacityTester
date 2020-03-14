#include <Arduino.h>
#include <GyverButton.h>
#include <LiquidCrystal_I2C.h>
#include <avr/eeprom.h>

#define PREFS 1

const char *menu[] = {"End mode", "End value"};
const char *end_modes[] = {"current", "voltage", "none"};

struct Prefs {
  byte end_mode;
  int end_value;
};

byte pref = 0;

void prepare_prefs_eeprom() {
  Prefs prefs;
  prefs.end_mode = 2;
  prefs.end_value = 0;

  eeprom_update_block((void*) &prefs, PREFS, sizeof(prefs));
}

void draw(LiquidCrystal_I2C& lcd, Prefs& prefs) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(menu[pref]);
  lcd.setCursor(0, 1);
  if (pref == 0) lcd.print(end_modes[prefs.end_mode]);
  else if (pref == 1) lcd.print(prefs.end_value);
}

void in(LiquidCrystal_I2C& lcd, GButton& button) {
  Prefs prefs;
  prefs.end_mode = 2;
  prefs.end_value = 0;

  eeprom_read_block((void*) &prefs, PREFS, sizeof(prefs));
  
  draw(lcd, prefs);
  for (;;) {
    button.tick();

    if (button.isSingle()) {
      if (pref == 0) {
        prefs.end_mode += 1;
        if (prefs.end_mode >= 3) prefs.end_mode = 0;
      } else if (pref == 1) {
        prefs.end_value += 100;
      }
      draw(lcd, prefs);
    } else if (button.isDouble()) {
      if (pref == 1) {
        prefs.end_value += 1000;
        draw(lcd, prefs);
      }
    } else if (button.isHolded()) {
      pref += 1;
      if (pref >= 2) pref = 0;
      draw(lcd, prefs);
    } else if (button.isTriple()) {
      eeprom_update_block((void*) &prefs, PREFS, sizeof(prefs));
      break;
    }
    
  }
  
  
}
