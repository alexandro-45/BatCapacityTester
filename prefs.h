#define PREFS 1 //адрес в памяті, з якого починаються настройки

#define PREF_ENDMODE 0
#define PREF_ENDVALUE 1

#define ENDMODE_CURRENT 0
#define ENDMODE_VOLTAGE 1
#define ENDMODE_NONE 3

const char *menu[] = {"End mode", "End value"}; //заголовки пунктів
const char *end_modes[] = {"current", "voltage", "none"}; //назви режимів

struct Prefs { //структура з настройками
  int end_mode; //границя по току/напрузі/без границі
  int end_value; //граничне значення мВ/мА
  int current_1A; //калібровка тока
  int current_raw_offset; //колібровка тока
  int voltage1V; //калідровка напруги
};

void prepare_prefs_eeprom() { //очистка настройок на стандартні
  Prefs prefs;
  prefs.end_mode = 0; //границя по току
  prefs.end_value = 70; //70 мА
  prefs.current_1A = 31; //калібровка тока
  prefs.current_raw_offset = 510; //калібровка тока
  prefs.voltage1V = 206; //калібровка напруги

  eeprom_update_block((void*) &prefs, PREFS, sizeof(prefs));
}

Prefs getPrefs() { //прочитати настройки
  Prefs p;
  eeprom_read_block((void*) &p, PREFS, sizeof(p));
  return p;
}

class Preferences {
  int cur_pref = 0;
  int adj = 1;
  uint8_t adjI = 0;
  
  void draw(LCD& lcd, Prefs& prefs) {
    lcd.clear();

    lcd.setCursor(0, 0);// пишу назву пункта настройок
    lcd.print(menu[cur_pref]);

    lcd.setCursor(1, 0);// малюю саму настройку
    if (cur_pref == PREF_ENDMODE) {//коли настройка end_mode, то пишу який зараз режим
      lcd.print(end_modes[prefs.end_mode]);
    } else if (cur_pref == PREF_ENDVALUE) {//коли настройка граничного знач., 
      lcd.print(prefs.end_value);// то пишу саме значення

      lcd.setCursor(2, 0);// і значення, на яке воно буде змінюватись коли нажімати кнопки
      lcd.print("agj: +");
      lcd.print(adj);
    }
  }

  public:
  void start(LCD& lcd, GButton& up, GButton& down) {
    Prefs prefs; //структура з настройками
    eeprom_read_block((void*) &prefs, PREFS, sizeof(prefs)); //читаю настройки

    draw(lcd, prefs); //малюю кадр
    //малювати кадр треба, щоб не був зразу пустий екран
    //бо я його перемальовую тільки тоді, коли нажалась кнопка
    
    for (;;) { // цикл (так як loop())
      up.tick();
      down.tick();
  
      //раз нажав вверх міняю режим/добавляю значення
      //раз нажав вниз міняю режим в другу сторону/віднімаю значення
      //два раза вверх - переходжу на пункт вверх
      //два раза вниз - переходжу на пункт вниз
      //зажімаю вверх - вийти і зберегти
      //зажімаю вниз - додавати більше/менше
      if (up.isSingle()) {
        //Serial.println("single up");
        if (cur_pref == PREF_ENDMODE) {
          prefs.end_mode += 1;
          if (prefs.end_mode >= 3) prefs.end_mode = 0;
        } else if (cur_pref == PREF_ENDVALUE) {
          prefs.end_value += adj;
        }
        draw(lcd, prefs);
      }
      if (up.isDouble()) {
        //Serial.println("double up");
        cur_pref += 1;
        if (cur_pref >= 2) cur_pref = 0;
        draw(lcd, prefs);
      }
      if (up.isHolded()) {
        //Serial.println("holded up");
        eeprom_update_block((void*) &prefs, PREFS, sizeof(prefs));
        break;
      }
      
      if (down.isSingle()) {
        //Serial.println("single down");
        if (cur_pref == PREF_ENDMODE) {
          prefs.end_mode -= 1;
          if (prefs.end_mode <= -1) prefs.end_mode = 2;
        } else if (cur_pref == PREF_ENDVALUE) {
          prefs.end_value -= adj;
        }
        draw(lcd, prefs);
      }
      if (down.isDouble()) {
        //Serial.println("double down");
        cur_pref -= 1;
        if (cur_pref <= -1) cur_pref = 1;
        draw(lcd, prefs);
      }
      if (down.isHolded()) {
        //Serial.println("holded down");
        adjI++;
        adj = pow(10, adjI % 4);
        draw(lcd, prefs);
      }
    }
  }
};
