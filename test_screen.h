#define DELAY 1 // час між замірами ємності в секундах
#define CURRENT_VALUES_COUNT 5 //к-ть замірів для обчислення середнього значення струму

class TestScreen: public Screen {
  private:
    bool exit_flag = false;
    
    long start_time = 0;     //коли почав тест
    long last_point = 0;     //час останнього заміру ємності
    long last_point_cur = 0; //час останнього заміру струму

    float capacity = 0;   //ємність в Ah
    float capacityWh = 0; //ємність в Wh

    float prev_current; //струм останнього заміру
    
    uint8_t current_values_counter = 0;         //лічильник замірів струму
    float current_values[CURRENT_VALUES_COUNT]; //5 останніх замірів струму

    bool bat_state = false; //чи була намальована батарейка в попередньому кадрі

    //обчислює середнє значення замірів струму
    float getCurCVal() {
      float val = 0;
      for (int i = 0; i < CURRENT_VALUES_COUNT; i++) {
        val += current_values[i];
      }
      return val / (float) CURRENT_VALUES_COUNT;
    }

    void draw(float current, float voltage, LCD& lcd) {
      lcd.setCursor(0, 0);
      lcd.print("Current: ");
      lcd.print(current);
      lcd.print("A");
    
      lcd.setCursor(1, 0);
      lcd.print("Capacity: ");

      lcd.setCursor(2, 0);
      lcd.print(capacity);
      lcd.print("Ah");
    
      lcd.setCursor(3, 0);
      lcd.print("          ");
      lcd.print(capacityWh);
      lcd.print("Wh");
    
      lcd.setCursor(4, 0);
      lcd.print("Voltage: ");
      lcd.print(voltage);
      lcd.print("V");
    
      lcd.setCursor(7, 101 - 6 * 4);
      lcd.print("Stop");
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
        backlightOn();
      }
    }
    
    void down_btn(GButton& down) {
      if (down.isSingle()) {
        backlightOn();
        exit_flag = true;
      }
    }
    
    void begin() {
      digitalWrite(USB_D_PULLDOWN, getVbat() > 2.7);
      exit_flag = false;
      start_time = millis();
      last_point = millis();
      capacity = 0;
      capacityWh = 0;
      prev_current = getCurrentA();
      current_values_counter = 0;
      bat_state = false;
    }
    
    void _do(LCD& lcd) {
        //замір струму (5 разів в секунду)
        if ((millis() - last_point_cur) >= (DELAY * 1000) / CURRENT_VALUES_COUNT) {
          last_point_cur = millis();
          current_values[current_values_counter] = getCurrentA();
          current_values_counter++;
          if (current_values_counter >= CURRENT_VALUES_COUNT) current_values_counter = 0;
        }

        //замір ємності (1 раз в секунду)
        if ((millis() - last_point) >= DELAY * 1000) {
          float current = getCurCVal();                                       //зчитати значення струму в даний момент (Iтеп[А])
          float current_s = (prev_current + current) / 2;                     //обчислити середнє значення струму (Iсер[А] = (Iпопер[А] + Iтеп[А]) / 2)
          
          float voltage = getVbat();                                          //зчитати значення напруги акумулятора в даний момент (V[В])
          if (voltage < 2.7) voltage = getVbus();                             //якщо напруга акумулятора менша за 2.7В, то акумулятор розряджений або не підключений, а значить живлення надходить по usb
          
          float add_cap = current_s * (float) ((float) DELAY / (float) 3600); //обчислити ємність в А*год на проміжку часу (С[А*год] = Iсер[A] * t[год])
          
          capacity += add_cap;                                                //додати ємність в А*год на проміжку часу до загальної
          capacityWh += add_cap * voltage;                                    //обчислити ємність в Вт*год на проміжку часу і додати до загальної (С[Вт*год] = C[А*год] * V[В])
      
          switch (prefs.stop_by) {                          //перевірити чи не пора завершити тест
            case STOP_BY_CURRENT:                           //якщо зупинка по струму
              if (current_s <= (prefs.stop_value / 1000)) { //перевірити чи струм менше границі
                exit_flag = true;                           //якщо менше, то завершити тест
              }
              break;
            case STOP_BY_VOLTAGE:                           //якщо зупинка по напрузі
              if (getVbat() < prefs.stop_value / 1000) {    //перевірити чи напруга менше границі
                exit_flag = true;                           //якщо менше, то завершити тест
              }
              break;
          }
      
          prev_current = current;      //записати струм
          last_point = millis();       //і час заміру
          draw(current, voltage, lcd); //намалювати екран
        }

        if (exit_flag) { //записати результат, якщо тест завершився
          HistEntry result;
          result.capacity = capacity;
          result.capacityWh = capacityWh;
          result.time = millis() - start_time;
        
          write_history(result);
        }
    }
    
    uint8_t switch_to() {
      return exit_flag ? RESULT_SCREEN : 255;
    }
};
