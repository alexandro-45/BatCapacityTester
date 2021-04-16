class CalibrationScreen: public Screen {
  private:
    uint8_t cursor = 0;
    uint8_t count = 4;
    const char *menu[4] = {"Raw off", "Cur 1A", "Bat 1V", "Bus 1V"};
    long last_draw = 0;
    int add_val = 0;
    bool saved = false;
    
    void draw(LCD& lcd) {
      for (uint8_t i = 0; i < count; i++) {
        lcd.clearLine(i);
        lcd.setCursor(i, 0);
        lcd.print(i == cursor ? ">" : " ");
        lcd.print(menu[i]);
        lcd.print(":");
        switch (i) {
          case 0:
            lcd.print(settings.current_raw_offset);
            break;
          case 1:
            lcd.print(settings.current_1A);
            break;
          case 2:
            lcd.print(settings.voltage1V_Vbat);
            break;
          case 3:
            lcd.print(settings.voltage1V_Vbus);
            break;
        }
      }
      lcd.setCursor(count, 0);
      lcd.print("Vbat: ");
      lcd.print(getVbat());

      lcd.setCursor(count + 1, 0);
      lcd.print("Vbus: ");
      lcd.print(getVbus());

      lcd.setCursor(count + 2, 0);
      lcd.print("Cur: ");
      lcd.print(getCurrentA());

      lcd.setCursor(7, 0);
      lcd.print("add val: ");
      lcd.print(pow(10, add_val % 4));

      if (saved) {
        lcd.setCursor(7, 90);
        lcd.print("*");
      }
    }
    
  public:
    void up_btn(GButton& up) {
      if (up.isSingle()) {
        cursor = cursor == 0 ? count - 1 : cursor - 1;
        last_draw = 0;
        backlightOn();
      } else if (up.isHolded()) {
        switch (cursor) {
          case 0:
            settings.current_raw_offset += pow(10, add_val % 4);
            break;
          case 1:
            settings.current_1A += pow(10, add_val % 4);
            break;
          case 2:
            settings.voltage1V_Vbat += pow(10, add_val % 4);
            break;
          case 3:
            settings.voltage1V_Vbus += pow(10, add_val % 4);
            break;
        }

        last_draw = 0;
        backlightOn();
      } else if (up.isDouble()) {
        add_val++;
        last_draw = 0;
        backlightOn();
      }
    }

    void down_btn(GButton& down) {
      if (down.isSingle()) {
        cursor = cursor == count - 1 ? 0 : cursor + 1;
        last_draw = 0;
        backlightOn();
      } else if (down.isHolded()) {
        switch (cursor) {
          case 0:
            settings.current_raw_offset -= pow(10, add_val % 4);
            break;
          case 1:
            settings.current_1A -= pow(10, add_val % 4);
            break;
          case 2:
            settings.voltage1V_Vbat -= pow(10, add_val % 4);
            break;
          case 3:
            settings.voltage1V_Vbus -= pow(10, add_val % 4);
            break;
        }
        last_draw = 0;
        backlightOn();
      } else if (down.isDouble()) {
        eeprom_update_block((void*) &settings, EEPROM_SETT_ADDR, sizeof(settings));
        saved = true;
        last_draw = 0;
        backlightOn();
      }
    }
    
    void _do(LCD& lcd) {
      if ((millis() - last_draw) > 1000) {
        last_draw = millis();
        draw(lcd);
      }
    }
};
