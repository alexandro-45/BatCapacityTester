class EndScreen: public Screen {
  private:
    bool exit_flag = false;
    bool isEmpty = false;
    bool redraw = false;
    HistEntry result;

    void draw(LCD& lcd) {
      lcd.setCursor(0, 0);
      lcd.print("Capacity: ");

      lcd.setCursor(1, 0);
      lcd.print(result.capacity);
      lcd.print("Ah");
    
      lcd.setCursor(2, 0);
      lcd.print(result.capacityWh);
      lcd.print("Wh");
      
      lcd.setCursor(3, 0);
      lcd.print("Time: ");
      lcd.print(formatTime(result.time));
    
      lcd.setCursor(7, 75);
      lcd.print("Exit");
    }
    
  public:
    void up_btn(GButton& up) {
      if (up.isSingle()) {
        exit_flag = true;
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
      exit_flag = false;
      isEmpty = false;
      redraw = false;
      int count = eeprom_read_byte(EEPROM_HISTORY_ADDR + ENTRIES_COUNT_OFFSET);//зчитую скільки записів в історії
      if (count != 0) eeprom_read_block((void*) &result, EEPROM_HISTORY_ADDR + ENTRIES_DATA_OFFSET + (count - 1) * sizeof(result), sizeof(result));
      else isEmpty = true;
    }
    
    void _do(LCD& lcd) {
      if (redraw) return;
      redraw = true;
      if (isEmpty) {
        lcd.setCursor(0, 0);
        lcd.print("Error, history is empty");
      }
      else draw(lcd);
    }
    
    uint8_t switch_to() {
      return exit_flag ? START_SCREEN : 255;
    }
};
