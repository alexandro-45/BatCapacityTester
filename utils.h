float getCurrentA() {
  int clear_val = analogRead(CURRENT_PIN) - settings.current_raw_offset;
  if (clear_val < 0) clear_val = 0;
  return (float) clear_val / (float) settings.current_1A;
}

float getVbat() {
  int raw = analogRead(VBAT_PIN);
  return (float) raw / (float) settings.voltage1V_Vbat;
}

float getVbus() {
  int raw = analogRead(VBUS_PIN);
  return (float) raw / (float) settings.voltage1V_Vbus;
}

String formatTime(long millis) {
  if (millis < 3600000) {
    String s = String(millis / 60000) + "min";
    return s;
  } else {
    int h = millis / 3600000;
    int min = (millis - h * 3600000) / 60000;
    String s = String(h) + "h " + String(min) + "min";
    return s;
  }
}
