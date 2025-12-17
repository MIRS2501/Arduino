void io_open() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_SW, INPUT);
  pinMode(PIN_BATT, INPUT);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_SW, HIGH);
  digitalWrite(PIN_BATT, LOW);
  pinMode(PIN_SW_1, INPUT_PULLUP);
  pinMode(PIN_SW_2, INPUT_PULLUP);
  pinMode(PIN_SW_3, INPUT_PULLUP);
}

void io_set_led(int val) {
  digitalWrite(PIN_LED, val);
}

int io_get_led() {
  return digitalRead(PIN_LED);
}

int io_get_sw() {
  return digitalRead(PIN_SW);
}

double io_get_batt() {
  return analogRead(PIN_BATT) * 5.0 / 1024.0 / V_RATIO;
}

int io_get_sw1() {
  return (digitalRead(PIN_SW_1) == LOW) ? 1 : 0;
}

int io_get_sw2() {
  return (digitalRead(PIN_SW_2) == LOW) ? 1 : 0;
}

int io_get_sw3() {
  return (digitalRead(PIN_SW_3) == LOW) ? 1 : 0;
}
