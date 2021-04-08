#define OFFSET 100 //офсет області для історії
#define COUNT_OFFSET 0 //офсет змінної, в якій записана кількість записів в історії
#define DATA_OFFSET 1 //офсет данних (початок данних)

struct Result {// запис в історії
  float capacity;
  float capacityWh;
  long time;  
};

void prepare_history_eeprom() {// обнулити кількість записів. тим самим очистити всі
  eeprom_update_byte(OFFSET + COUNT_OFFSET, 0);
}

void write_history(Result& result) {// додати запис в історію
  int count = eeprom_read_byte(OFFSET + COUNT_OFFSET);
  if (count >= 9) count = 0;

  eeprom_update_block((void*) &result, OFFSET + DATA_OFFSET + (count * sizeof(result)), sizeof(result));
  eeprom_update_byte(OFFSET + COUNT_OFFSET, count + 1);
  count++;
}

class History {
  uint8_t cursor = 0;
  uint8_t count = 0;
  
  void draw(LCD& lcd, Result& result) {
    lcd.clear();
    
    lcd.setCursor(0, 0);// capacity
    lcd.print("Capacity: ");
    lcd.print(result.capacity);
    lcd.print("Ah");

    lcd.setCursor(1, 0);
    lcd.print(result.capacityWh);
    lcd.print("Wh");
  
    lcd.setCursor(2, 0);// time
    lcd.print("Time: ");
    lcd.print(result.time / 3600000);
    lcd.print("h");
  
    lcd.setCursor(7, 0); //page (cursor)
    lcd.print(cursor);
    
    lcd.setCursor(7, 75);//text "exit"
    lcd.print("Exit");
  }

  public:
  void start(LCD& lcd, GButton& up_btn, GButton& down_btn) {
    count = eeprom_read_byte(OFFSET + COUNT_OFFSET);//зчитую скільки записів в історії
    if (count == 0) {// якщо їх немає то пишу, що історія чиста
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("history is clean");
      lcd.setCursor(1, 0);
      lcd.print("Press to exit");
    } else {//якщо записи є, то читаю перший і малюю
      Result r;
      eeprom_read_block((void*) &r, OFFSET + DATA_OFFSET + cursor * sizeof(r), sizeof(r));
      draw(lcd, r);
    }
  
    for (;;) {
      up_btn.tick();
      down_btn.tick();
  
      if (up_btn.isSingle()) {//нажав раз вверх
        if (count == 0) break;//нема записів, то нічого не робити
        else {// є, то переключити на слідуючий
          cursor++;
          if (cursor > count) cursor = 0;
          Result r;
          eeprom_read_block((void*) &r, OFFSET + DATA_OFFSET + cursor * sizeof(r), sizeof(r));
          draw(lcd, r);
        }
      }
      if (down_btn.isSingle()) {//нажав раз вниз, то вийти з історії
        break;
      }
    }
  }
};
