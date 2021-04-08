#define PREF_RAWOFFSET 0
#define PREF_1AVALUE 1
#define PREF_1VVALUE 2
#define PREFS_COUNT 3

#define BUFFER_SIZE 5

#define CURRENT_PIN 14 //для тока A0
#define VOLTAGE_PIN 15 //для напруги A1

class Calibration {
  Prefs prefs;
  int adj = 1;
  uint8_t adjI = 0;
  int pref = 0;
  int fps = 2;
  long last_draw = 0;
  int buffer[BUFFER_SIZE];
  long last_craw = 0;
  int buffer_counter = 0;
  int cval = 0;

  void getCval() {
    int sum = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) sum += buffer[i];
    cval = sum / BUFFER_SIZE;
  }

  float getCurrentA(int raw) {
    int clear_val = raw - prefs.current_raw_offset;
    if (clear_val < 0) clear_val = 0;
    return (float) clear_val / (float) prefs.current_1A;
  }
  
  float getVoltageV() {
    int raw = analogRead(VOLTAGE_PIN);
    return (float )raw / (float) prefs.voltage1V;
  }
  
  void draw(LCD& lcd, Prefs& prefs) {
    lcd.setCursor(0, 0);
    lcd.print("Calibration mode");

    //lcd.clearLine(1);
    lcd.setCursor(1, 0);
    lcd.print("Raw offset: ");
    lcd.print(prefs.current_raw_offset);
    if (pref == PREF_RAWOFFSET) lcd.print("<");

    //lcd.clearLine(2);
    lcd.setCursor(2, 0);
    lcd.print("1A value: ");
    lcd.print(prefs.current_1A);
    if (pref == PREF_1AVALUE) lcd.print("<");

    //lcd.clearLine(3);
    lcd.setCursor(3, 0);
    lcd.print("1V value: ");
    lcd.print(prefs.voltage1V);
    if (pref == PREF_1VVALUE) lcd.print("<");

    //lcd.clearLine(4);
    lcd.setCursor(4, 0);
    lcd.print("adj: +");
    lcd.print(adj);

    //lcd.clearLine(5);
    lcd.setCursor(5, 0);
    lcd.print("Voltage: ");
    lcd.print(getVoltageV());
    lcd.print("V");

    //lcd.clearLine(6);
    lcd.setCursor(6, 0);
    lcd.print("Current: ");
    lcd.print(getCurrentA(cval));
    lcd.print("A");
  }
  public:
  void start(LCD& lcd, GButton& up, GButton& down) {
    prefs = getPrefs();

    for(;;) {
      up.tick();
      down.tick();
  
      if ((millis() - last_draw) > (1000 / fps)) {
        draw(lcd, prefs);
      }

      up.tick();
      down.tick();

      if ((millis() - last_craw) > (1000 / BUFFER_SIZE)) {
        buffer[buffer_counter] = analogRead(CURRENT_PIN);
        buffer_counter++;
        if (buffer_counter > BUFFER_SIZE - 1) {
          buffer_counter = 0;
          getCval();
        }
      }

      up.tick();
      down.tick();
      
      if (up.isSingle()) {
        switch(pref) {
          case PREF_RAWOFFSET:
            prefs.current_raw_offset += adj;
            break;
          case PREF_1AVALUE:
            prefs.current_1A += adj;
            break;
          case PREF_1VVALUE:
            prefs.voltage1V += adj;
            break;
        }
      }
  
      if (down.isSingle()) {
        switch(pref) {
          case PREF_RAWOFFSET:
            prefs.current_raw_offset -= adj;
            break;
          case PREF_1AVALUE:
            prefs.current_1A -= adj;
            break;
          case PREF_1VVALUE:
            prefs.voltage1V -= adj;
        }
      }
  
      if (up.isDouble()) {
        pref--;
        if (pref < 0) pref = PREFS_COUNT - 1;
      }
  
      if (down.isDouble()) {
        pref++;
        if (pref > PREFS_COUNT - 1) pref = 0;
      }
  
      if (up.isHolded()) {
        eeprom_update_block((void*) &prefs, PREFS, sizeof(prefs));
        break;
      }
  
      if (down.isHolded()) {
        adjI++;
        adj = pow(10, adjI % 3);
      }
    }
  }
};
