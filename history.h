#define EEPROM_HISTORY_ADDR 100
#define ENTRIES_COUNT_OFFSET 0
#define ENTRIES_DATA_OFFSET 1

struct HistEntry {// запис в історії
  float capacity;
  float capacityWh;
  long time;  
};

void prepare_history_eeprom() {// обнулити кількість записів
  eeprom_update_byte(EEPROM_HISTORY_ADDR + ENTRIES_COUNT_OFFSET, 0);
}

void write_history(HistEntry& entry) {// додати запис в історію
  int count = eeprom_read_byte(EEPROM_HISTORY_ADDR + ENTRIES_COUNT_OFFSET);
  if (count > 8) count = 0;//9

  eeprom_update_block((void*) &entry, EEPROM_HISTORY_ADDR + ENTRIES_DATA_OFFSET + (count * sizeof(entry)), sizeof(entry));
  eeprom_update_byte(EEPROM_HISTORY_ADDR + ENTRIES_COUNT_OFFSET, count + 1);
}

class HistoryScreen: public Screen {
  private:
    uint8_t cursor = 0;
    uint8_t count = 0; //entries count
    HistEntry entry;
    bool exit_flag = false;
    bool isEmpty = false;
    bool redraw = false;

    void draw(LCD& lcd) {
      lcd.clear();
      
      lcd.setCursor(0, 0);// capacity
      lcd.print("Capacity: ");
      
      lcd.setCursor(1, 0);
      lcd.print(entry.capacity);
      lcd.print("Ah");
  
      lcd.setCursor(2, 0);
      lcd.print(entry.capacityWh);
      lcd.print("Wh");
    
      lcd.setCursor(3, 0);// time
      lcd.print("Time: ");
      lcd.print(formatTime(entry.time));
    
      lcd.setCursor(7, 0); //page (cursor)
      lcd.print(cursor);

      lcd.setCursor(0, 75);//text "next"
      lcd.print("Next");
      
      lcd.setCursor(7, 75);//text "exit"
      lcd.print("Exit");
    }
  
  public:
    void up_btn(GButton& up) {
      if (up.isSingle()) {
        if (count != 0) {
          cursor++;
          if (cursor >= count) cursor = 0;
          eeprom_read_block((void*) &entry, EEPROM_HISTORY_ADDR + ENTRIES_DATA_OFFSET + cursor * sizeof(entry), sizeof(entry));
          redraw = false;
        }
        backlightOn();
      }
    }
    
    void down_btn(GButton& down) {
      if (down.isSingle()) {
        exit_flag = true;
        backlightOn();
      }
    }
    
    void begin() {
      cursor = 0;
      exit_flag = false;
      isEmpty = false;
      redraw = false;
      count = eeprom_read_byte(EEPROM_HISTORY_ADDR + ENTRIES_COUNT_OFFSET);
      if (count == 0) {
        isEmpty = true;
      } else {//read first
        eeprom_read_block((void*) &entry, EEPROM_HISTORY_ADDR + ENTRIES_DATA_OFFSET + cursor * sizeof(entry), sizeof(entry));
      }
    }
    
    void _do(LCD& lcd) {
      if (redraw) return;
      redraw = true;
      if (isEmpty) {
        lcd.setCursor(0, 0);
        lcd.print("history is empty");
        lcd.setCursor(1, 0);
        lcd.print("Press to exit");
      } else {
        draw(lcd);
      }
    }
    
    uint8_t switch_to() {
      return exit_flag ? START_SCREEN : 255;
    }
};
