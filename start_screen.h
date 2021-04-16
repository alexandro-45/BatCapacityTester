class StartScreen: public Screen {
  private:
    uint8_t _switch_to = 255;
    bool redraw = false;
    bool bat_state = false; //чи була намальована батарейка в попер. кадрі
    
    void draw(LCD& lcd) {
      lcd.clear();
      
      lcd.setCursor(0, 101 - 6 * 5);
      lcd.print("Start");

      lcd.setCursor(3, 6*3);
      lcd.print("Connect load");
      lcd.setCursor(4, 6);
      lcd.print("and press start");
    
      lcd.setCursor(7, 101 - 6 * 9);
      lcd.print("Hist/Sett");
    }
    
    void draw_bat(LCD& lcd) {
      if (getVbat() > 2.7 && !bat_state) {
        lcd.setCursor(7, 20);
        for (int i = 0; i < 6; i++) {
          lcd.sendData(pgm_read_byte(&battery[i]));
        }
        bat_state = true;
      } else if (getVbat() < 2.7 && bat_state) {
        lcd.setCursor(7, 20);
        lcd.print(' ');
        bat_state = false;
      }
    }
    
  public:
    void up_btn(GButton& up) {
      if (up.isSingle()) {
        _switch_to = TEST_SCREEN;
        backlightOn();
      }
    }
    
    void down_btn(GButton& down) {
      if (down.isSingle()) {
        _switch_to = HISTORY_SCREEN;
        backlightOn();
      } else if (down.isDouble()) {
        _switch_to = PREF_SCREEN;
        backlightOn();
      }
    }
    
    void begin() {
      _switch_to = 255;
      redraw = false;
      bat_state = false;
    }
    
    void _do(LCD& lcd) {
      if (!redraw) {
        redraw = true;
        draw(lcd);
      }
      draw_bat(lcd);
    }
    
    uint8_t switch_to() {
      return _switch_to;
    }
};
