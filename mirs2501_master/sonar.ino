void sonar_open() {
  pinMode(PIN_US_TRIG, OUTPUT);
  pinMode(PIN_US_ECHO, INPUT);
}

double sonar_get_dist() {
  // Trigger信号を送信 (10us)
  digitalWrite(PIN_US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_US_TRIG, LOW);

  // EchoピンがHIGHの時間を計測 (最大20ms = 約3.4m)
  // タイムアウトしたら計測不能として大きな値を返す
  double duration = pulseIn(PIN_US_ECHO, HIGH, 20000); 
  
  if (duration == 0) return 999.0; // 計測不能時は999cmとする
  
  // 距離 [cm] = 時間 [us] * 0.034 [cm/us] / 2
  return duration * 0.034 / 2.0;
}
