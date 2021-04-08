#include <LCD_101x64.h>
#include <GyverButton.h>
#include "prefs.h"
#include "history.h"
#include "calibration.h"

#define DELAY 1 //проміжок часу між замірами (в секундах)
#define CURRENT_VALUES_COUNT 5 //к-ть замірів
#define BACKLIGHT_ON 10000 // час, через який викл. підсвітку

//#define MOSFET_PIN 6 //мав бути мосфет
#define UP_BTN 2 //кнопка
#define DOWN_BTN 3 //додаткова кнопка
#define USB_D_PULLDOWN 4 //для стяжки usb D+, D-
#define BACKLIGHT_PIN 5 //для підсвітки дисплея
#define CURRENT_PIN 14 //для тока (A0)
#define VOLTAGE_PIN 15 //для напруги (A1)

LCD lcd(13, 12, 11, 10, 9);
GButton up_btn(UP_BTN);
GButton down_btn(DOWN_BTN);

bool run = 0; //чи запущений тест
uint8_t mode = 0; // 0 - default, 1 - settings, 2 - history
bool old_bat = 0; //чи була намальована батарейка в попер. кадрі

long start_time = 0; //коли начав тест
long last_point = 0; //останній замір
long last_point_cur = 0; //останній замір тока
long lastbacklightInterrupt = 0; //час, коли вкл. підсвітка

float capacity = 0; //ємність
float capacityWh = 0; //ємність в Wh

float prev_current; //ток останнього заміру
float current_values[CURRENT_VALUES_COUNT]; //5 останніх замірів

uint8_t current_values_counter = 0; //щотчик замірів

Prefs prefs; //просто настройки

//символ батарейки
const uint8_t battery[6] PROGMEM = {
  0b1111110,
  0b1111110,
  0b1111111,
  0b1111111,
  0b1111110,
  0b1111110
};

void setup() {
  //Serial.begin(9600);

  //pinMode(MOSFET_PIN, OUTPUT);
  pinMode(USB_D_PULLDOWN, OUTPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);

  if (eeprom_read_byte(0) != 222) { //якщо перший запуск то обнуляю
    prepare_history_eeprom();
    prepare_prefs_eeprom();
    eeprom_write_byte(0, 222);
    //Serial.println("first run");
  }

  prefs = getPrefs();

  lcd.init();
  delay(100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Capacity tester");
  
  if (!digitalRead(DOWN_BTN)) {
    Calibration c;
    c.start(lcd, up_btn, down_btn);
  }
  
  delay(1000);
  draw_main();
}

void loop() {
  //Serial.println("Im not died)");
  up_btn.tick();
  down_btn.tick();
  backlightTick();

  //має показувати символ батарейки коли підключений акум
  if (getVoltageV() > 2.7) {// якщо підкл. акум і раніше не малювався то намалювати його
    if (!old_bat) {
      lcd.setCursor(7, 20);
      for (int i = 0; i < 6; i++) {
        lcd.sendData(pgm_read_byte(&battery[i]));
      }
      old_bat = 1;
    }
  } else { //якщо відкл. акум, але раніше малювався то стерти його
    if (old_bat) {
      lcd.setCursor(7, 20);
      lcd.print(' ');
      old_bat = 0;
    }
  }

  if (up_btn.isSingle() && !run) { //нажав раз - запустити тест
    backlightInterrupt();//включити підсвітку
    digitalWrite(USB_D_PULLDOWN, getVoltageV() > 2.7);
    start_time = millis();
    last_point = millis();
    capacity = 0;
    capacityWh = 0;
    prev_current = getCurrentA();
    run = 1;
    lcd.clear();
  }

  if (down_btn.isSingle()) { //нажав раз на другу кнопку - відкрити історію
    backlightInterrupt();
    if (!run) {
      mode = 2;
      History h;
      h.start(lcd, up_btn, down_btn);
      mode = 0;
  
      draw_main();
    } else { //а якщо запущений тест - завершити
      run = 0;
      _end();
    }
  } else if (down_btn.isHolded() && !run) { //потримав другу - відкрити настройки
    backlightInterrupt();
    mode = 1;
    Preferences p;
    p.start(lcd, up_btn, down_btn);
    mode = 0;

    draw_main();
  }

  //заміряю ток 5 раз в секунду
  if (mode == 0 && run && (millis() - last_point_cur) >= (DELAY * 1000) / CURRENT_VALUES_COUNT) {
    last_point_cur = millis();
    current_values[current_values_counter] = getCurrentA();
    current_values_counter++;
    if (current_values_counter >= CURRENT_VALUES_COUNT) current_values_counter = 0;
  }

  if (mode == 0 && run && (millis() - last_point) >= DELAY * 1000) {
    float current = getCurCVal();
    float voltage  = getVoltageV();
    if (voltage < 2.7) voltage = 5;
    float current_s = (prev_current + current) / 2; // середній ток на проміжку

    float adj_cap = current_s * (float) ((float) DELAY / (float) 3600); // ємність на проміжку

    capacity += adj_cap; // добавав ємність на проміжку до загальної
    capacityWh += adj_cap * voltage; // добавив ємність на проміжку до загальної (Wh)

    switch (prefs.end_mode) {                         //тут перевіряю чи не пора закінчити тест
      case ENDMODE_CURRENT:                           //якщо границя по току
        if (current_s <= (prefs.end_value / 1000)) {  //перевіряю чи ток не менше границі
          _end();                                     //обробка результатів
        }
        return;                                       //викидаю на слідуючий цикл, бо внизу обновляю екран
      case ENDMODE_VOLTAGE:                           //якщо границя по напрузі
        if (getVoltageV() < prefs.end_value / 1000) { //перевіряю чи напруга не менше границі
          _end();                                     //обробка результатів
        }
        return;                                       //викидаю на слідуючий цикл, бо внизу обновляю екран
    }

    prev_current = current; //запоминаю ток
    last_point = millis();  //і час заміру
    draw(current, voltage); //малюю
  }
}

//малює екран коли тест закінчився
void _end() {
  run = 0;
  //записую результати в структуру
  Result result;
  result.capacity = capacity;
  result.capacityWh = capacityWh;
  result.time = millis() - start_time;

  //записую структуру в память
  write_history(result);

  //малюю
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Capacity: ");
  lcd.print(capacity);
  lcd.print("Ah");

  lcd.setCursor(1, 0);
  lcd.print(capacityWh);
  lcd.print("Wh");
  
  lcd.setCursor(2, 0);
  lcd.print("Time: ");
  //lcd.print((millis() - start_time) / 3600000);
  lcd.print(formatTime(millis() - start_time));
  lcd.print("h");

  lcd.setCursor(3, 0);
  lcd.print("Connect battery");
  lcd.setCursor(4, 0);
  lcd.print("and press start");

  lcd.setCursor(7, 101 - 6 * 7);
  lcd.print("History");
}

//малює екран під час тесту
void draw(float current, float voltage) {
  lcd.setCursor(0, 0);
  lcd.print("Current: ");
  lcd.print(current);
  lcd.print("A");

  lcd.setCursor(1, 0);
  lcd.print("Capacity: ");
  lcd.print(capacity);
  lcd.print("Ah");

  lcd.setCursor(2, 0);
  lcd.print("          ");
  lcd.print(capacityWh);
  lcd.print("Wh");

  lcd.setCursor(3, 0);
  lcd.print("Voltage: ");
  lcd.print(voltage);
  lcd.print("V");

  lcd.setCursor(7, 101 - 6 * 4);
  lcd.print("Stop");
}

//малює головний екран
void draw_main() {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Connect battery");
  lcd.setCursor(1, 0);
  lcd.print("and press start");

  lcd.setCursor(7, 101 - 6 * 7);
  lcd.print("History");
}

//обчислює середнє значення замірів току
float getCurCVal() {
  float val = 0;
  for (int i = 0; i < CURRENT_VALUES_COUNT; i++) {
    val += current_values[i];
  }
  return val / (float) CURRENT_VALUES_COUNT;
}
//получає значення з аналогового піна
//і перетворює його в значення сили тока в амперах
float getCurrentA() {
  int clear_val = analogRead(CURRENT_PIN) - prefs.current_raw_offset;
  if (clear_val < 0) clear_val = 0;
  return (float) clear_val / (float) prefs.current_1A;
}

//дає напругу в Вольтах
float getVoltageV() {
  int raw = analogRead(VOLTAGE_PIN);
  return (float )raw / (float) prefs.voltage1V;
}

//включити підсвітку
void backlightInterrupt() {
  lastbacklightInterrupt = millis();
  digitalWrite(BACKLIGHT_PIN, HIGH);
}

//перевіряє шо там по таймеру підсвітки і вирубає якшо треба
void backlightTick() {
  if (millis() - lastbacklightInterrupt > BACKLIGHT_ON) {
    digitalWrite(BACKLIGHT_PIN, LOW);
  }
}

const char* formatTime(long millis) {
  if (millis < 3600000) {
    String s = String(millis / 60000) + "min";
    return s.c_str();
  } else {
    int h = millis / 3600000;
    int min = (millis - h * 3600000) / 60000;
    String s = String(h) + "h " + String(min) + "min";
    return s.c_str();
  }
}
