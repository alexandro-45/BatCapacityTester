#include <GyverButton.h>
#include <LiquidCrystal_I2C.h>
#include <avr/eeprom.h>
#include "prefs.h"
#include "history.h"

#define _1A 35 //константа для перевода рівня сигнала в струм
#define DELAY 1 //проміжок часу між замірами (в секундах)
#define CURRENT_RAW_OFFSET 510 //відсікає постійне значення сигнаала

#define CURRENT_PIN 14 //для тока
#define MOSFET_PIN 3 //мав бути мосфет
#define BTN 2 //кнопка
#define VOLTAGE_PIN 15

#define END_MODE 0 //0 - ток, 1 - напруга, 2 - нема
#define END_VAL 70 // ток - мА, напруга - мВ

LiquidCrystal_I2C lcd(0x20,16,2);
GButton button(BTN);

bool run = 0;
uint8_t mode = 0; // 0 - default, 1 - settings

volatile long start_time = 0;
volatile long last_point = 0;

float capacity = 0;

float prev_current;

void setup() {
  Serial.begin(9600);
  pinMode(MOSFET_PIN, OUTPUT);

  pinMode(A0, OUTPUT);
  pinMode(A2, OUTPUT);
  digitalWrite(A0, HIGH);
  digitalWrite(A2, LOW);
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  if (eeprom_read_byte(0) != 222) {
    prepare_history_eeprom();
    prepare_prefs_eeprom();
    eeprom_write_byte(0, 222);
    Serial.println("first run");
  }
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Capacity tester");

  delay(1000);
  
  lcd.setCursor(0, 0);
  lcd.print("Connect battery");
  lcd.setCursor(0, 1);
  lcd.print("and press start");
}

void loop() {
  //Serial.println("Im not died)");
  //Serial.print("Run is "); Serial.println(run);
  button.tick();

  if (button.isSingle() && !run) {
    start_time = millis();
    last_point = millis();
    prev_current = getCurrentA();
    run = 1;
  } else if (button.isDouble() && !run) {
    mode = 2;
    inn(lcd, button);
    mode = 0;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connect battery");
    lcd.setCursor(0, 1);
    lcd.print("and press start");
  } else if (button.isTriple()) {
    prepare_history_eeprom();
  }

  if (button.isHolded() && !run) {
    mode = 1;
    in(lcd, button);
    mode = 0;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connect battery");
    lcd.setCursor(0, 1);
    lcd.print("and press start");
  }
  
  if (mode == 0 && run && (millis() - last_point) >= DELAY * 1000) {
    float current = getCurrentA(); // ток зараз
    float current_s = (prev_current + current) / 2; // середній ток на проміжку
    
    //Serial.print("cur: ");
    //Serial.println(current);
    
    //Serial.print("cur_s: ");
    //Serial.println(current_s);
    
    float adj_cap = current_s * (float) ((float) DELAY / (float) 3600); // ємність на проміжку
    
    //Serial.print("adj: ");
    //Serial.println(adj_cap);
    
    capacity += adj_cap; // добавав ємність на проміжку до загальної

    switch (END_MODE) {
      case 0:
        if (current_s <= (END_VAL / 1000)) {
          run = 0;
          _end();
          return;
        }
        break;
      case 1:
        if (analogRead(VOLTAGE_PIN) <= (END_VAL / 4.8828125)) {
          run = 0;
          _end();
        }
        break;
      default:
        break;
    }

    /////записую 
    prev_current = current;
    last_point = millis();
    draw(current);
  }
}

void _end() {
  run = 0;

  Result result;
  result.capacity = capacity;
  result.time = millis() - start_time;

  write_history(result);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Capacity: ");
  lcd.print(capacity);
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print((millis() - start_time) / 3600000);
  lcd.print("h");
}

void draw(float current) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Current: ");
  lcd.print(current);
  lcd.print("A");
  
  lcd.setCursor(0, 1);
  lcd.print("Capacity: ");
  lcd.print(capacity);
  lcd.print("Ah");
}

float getCurrentA() {
  int clear_val = analogRead(CURRENT_PIN) - CURRENT_RAW_OFFSET;
  if (clear_val < 0) clear_val = 0;
  return (float) clear_val / (float) _1A;
}
