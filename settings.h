#define EEPROM_SETT_ADDR 1

#define PREF_STOP_BY 0
#define PREF_STOP_VALUE 1

#define STOP_BY_CURRENT 0
#define STOP_BY_VOLTAGE 1
#define STOP_BY_NONE 2

struct Settings {
  int stop_by; //зупинка по струму/напрузі/без зупинки
  int stop_value; //граничне значення мВ/мА
  int current_1A; //калібровка струму
  int current_raw_offset; //калібровка струму
  int voltage1V_Vbat; //калібровка напруги vbat
  int voltage1V_Vbus; //калібровка напруги vbus
} settings;

void prepare_settings_eeprom() {//стандартні налаштування
  Settings settings;
  settings.stop_by = STOP_BY_CURRENT;
  settings.stop_value = 70; //70 мА
  settings.current_1A = 31;
  settings.current_raw_offset = 510;
  settings.voltage1V_Vbat = 206;
  settings.voltage1V_Vbus = 57;

  eeprom_update_block((void*) &settings, EEPROM_SETT_ADDR, sizeof(settings));
}

void getSettings() {
  eeprom_read_block((void*) &settings, EEPROM_SETT_ADDR, sizeof(settings));
}

class PrefScreen: public Screen {
  private:
    bool redraw = true;
    uint8_t cursor = 0;
    uint8_t count = 2; //settings count
    uint8_t add_val = 0; //
    bool exit_flag = false;
    
    const char *menu[2] = {"Stop by", "Stop value"};
    const char *stop_values[3] = {"current", "voltage", "none"};

    void draw_main(LCD& lcd) {
      for (uint8_t i = 0; i < count; i++) {
        lcd.clearLine(i);
        lcd.setCursor(i, 0);
        lcd.print(i == cursor ? ">" : " ");
        lcd.print(menu[i]);
        lcd.print(" ");
        if (i == 0) {
          lcd.print(stop_values[settings.stop_by]);
        } else if (i == 1) {
          lcd.print(settings.stop_value);
        }
      }
      lcd.clearLine(7);
      lcd.setCursor(7, 0);
      lcd.print("add: ");
      lcd.print(pow(10, add_val % 4), 0);

      lcd.setCursor(7, 101 - 6 * 4);
      lcd.print("Save");
//
//      lcd.setCursor(0, 101 - 6 * 3);
//      lcd.print("add");
    }

  public:
    void up_btn(GButton& up) {
      if (up.isSingle()) {
        cursor = cursor == 0 ? count - 1 : cursor - 1;
        redraw = true;
        backlightOn();
      } else if (up.isHolded()) {
        if (cursor == 0) {
          settings.stop_by = settings.stop_by == 2 ? 0 : settings.stop_by + 1;
        } else if (cursor == 1) {
          settings.stop_value += pow(10, add_val % 4);
        }
        redraw = true;
        backlightOn();
      } else if (up.isDouble()) {
        add_val++;
        redraw = true;
        backlightOn();
      }
    }
    
    void down_btn(GButton& down) {
      if (down.isSingle()) {
        cursor = cursor == count - 1 ? 0 : cursor + 1;
        redraw = true;
        backlightOn();
      } else if (down.isHolded()) {
        if (cursor == 0) {
          settings.stop_by = settings.stop_by == 0 ? 2 : settings.stop_by - 1;
        } else if (cursor == 1) {
          settings.stop_value -= pow(10, add_val % 4);
        }
        redraw = true;
        backlightOn();
      } else if (down.isDouble()) {
        eeprom_update_block((void*) &settings, EEPROM_SETT_ADDR, sizeof(settings));
        exit_flag = true;
        backlightOn();
      }
    }
    
    void begin() {
      redraw = true;
      cursor = 0;
      exit_flag = false;
    }
    
    void _do(LCD& lcd) {
      if (redraw) {
        draw_main(lcd);
        redraw = false;
      }
    }
    
    uint8_t switch_to() {
      return exit_flag ? START_SCREEN : 255;
    }
};
