#include <LiquidCrystal_I2C.h>

#define _1A 35 //константа для перевада рівня сигнала в струм
#define DELAY 1 //проміжок часу між замірами (в секундах)
#define CURRENT_RAW_OFFSET 510 //відсікає постійне значення сигнаала

#define CURRENT_PIN 18 //для тока
#define MOSFET_PIN 3 //мав бути мосфет
#define START_BTN_INT 0 //стартова кнопка

LiquidCrystal_I2C lcd(0x20,16,2);

bool run = 0;

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
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Capacity tester");

  delay(1000);
  
  lcd.setCursor(0, 0);
  lcd.print("Connect battery");
  lcd.setCursor(0, 1);
  lcd.print("and press start");

  attachInterrupt(0, _startInterrupt, FALLING);
}

void loop() {
  if (run && (millis() - last_point) >= DELAY * 1000) {
    float current = getCurrentA(); // ток зараз
    float current_s = (prev_current + current) / 2; // середній ток на проміжку
    
    Serial.print("cur: ");
    Serial.println(current);
    
    Serial.print("cur_s: ");
    Serial.println(current_s);
    
    float adj_cap = current_s * (float) ((float) DELAY / (float) 3600); // ємність на проміжку
    
    Serial.print("adj: ");
    Serial.println(adj_cap);
    
    capacity += adj_cap; // добавав ємність на проміжку до загальної

//    if (current_s < 0.07) { // якщо ток на проміжку менше 70мА, то вирубаю
//      _end();
//    }

    /////записую 
    prev_current = current;
    last_point = millis();
    draw(current);
  }
}

void _end() {
  run = 0;
  lcd.setCursor(0, 0);
  lcd.print("Capacity: ");
  lcd.print(capacity);
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print((millis() - start_time) / 3600000);
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
  int clear_val = analogRead(A1) - CURRENT_RAW_OFFSET;
  if (clear_val < 0) clear_val = 0;
  //Serial.println(analogRead(A1));
  //Serial.println(clear_val);
  //Serial.println((float) clear_val / (float) _1A);
  return (float) clear_val / (float) _1A;
}

void _startInterrupt() {
  start_time = millis();
  last_point = millis();
  prev_current = getCurrentA();
  run = 1;
  detachInterrupt(0);
}
