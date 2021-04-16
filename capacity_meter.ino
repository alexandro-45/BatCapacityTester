#include <LCD_101x64.h>
#include <GyverButton.h>

#define UP_BTN 2
#define DOWN_BTN 3
#define USB_D_PULLDOWN 4 //для стяжки usb D+, D-
#define BACKLIGHT_PIN 5
#define CURRENT_PIN 14
#define VBAT_PIN 15
#define VBUS_PIN 16

#define BACKLIGHT_TIMEOUT 10000

//#define USE_SERIAL

#ifdef USE_SERIAL
  #define DEBUG_PRINTLN(X) Serial.println(X);
  #define DEBUG_PRINT(X) Serial.print(X);
#else
  #define DEBUG_PRINTLN(X) {}
  #define DEBUG_PRINT(X) {}
#endif

//символ батарейки
const uint8_t battery[6] PROGMEM = {
  0b1111110,
  0b1111110,
  0b1111111,
  0b1111111,
  0b1111110,
  0b1111110
};

long lastbacklightInterrupt = 0; //час, коли вкл. підсвітка

void backlightTick() {
  if (millis() - lastbacklightInterrupt > BACKLIGHT_TIMEOUT) {
    digitalWrite(BACKLIGHT_PIN, LOW);
  }
}

void backlightOn() {
  lastbacklightInterrupt = millis();
  digitalWrite(BACKLIGHT_PIN, HIGH);
}

#include "screen.h"
#include "settings.h"
#include "utils.h"
#include "history.h"
#include "end_screen.h"
#include "start_screen.h"
#include "test_screen.h"
#include "calibration.h"

LCD lcd(13, 12, 11, 10, 9);
GButton up_btn(UP_BTN);
GButton down_btn(DOWN_BTN);

uint8_t screen = START_SCREEN;
Screen* screens[6];

void addScreens() {
  screens[TEST_SCREEN] = new TestScreen();
  screens[RESULT_SCREEN] = new EndScreen();
  screens[START_SCREEN] = new StartScreen();
  screens[HISTORY_SCREEN] = new HistoryScreen();
  screens[PREF_SCREEN] = new PrefScreen();
  screens[CALIBRATION_SCREEN] = new CalibrationScreen();
}

void setup() {
  #ifdef USE_SERIAL
    Serial.begin(9600);
  #endif

  pinMode(USB_D_PULLDOWN, OUTPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);

  up_btn.setDebounce(50);
  up_btn.setTimeout(700);

  down_btn.setDebounce(50);
  down_btn.setTimeout(700);

  if (eeprom_read_byte(0) != 222) { //якщо перший запуск то обнуляю
    prepare_history_eeprom();
    prepare_settings_eeprom();
    eeprom_write_byte(0, 222);
    DEBUG_PRINTLN("first run");
  }

  getSettings();

  lcd.init();
  delay(100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Capacity tester");
  
  if (!digitalRead(DOWN_BTN)) {
    screen = CALIBRATION_SCREEN;
  }
  
  addScreens();
  backlightOn();
  
  delay(1000);
  screens[screen]->begin();
}

void loop() {
  backlightTick();
  up_btn.tick();
  down_btn.tick();
  
  screens[screen]->up_btn(up_btn);
  screens[screen]->down_btn(down_btn);
  screens[screen]->_do(lcd);

  if (screens[screen]->switch_to() != 255) {
    screen = screens[screen]->switch_to();
    lcd.clear();
    screens[screen]->begin();
  }
}
