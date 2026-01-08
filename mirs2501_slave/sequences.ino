#include "define.h"

// 外部参照
extern Servo s1, s2, s3;
extern AccelStepper stepperX, stepperY;
extern const long POS_X[];

void actuator_move(double target_rad);

// --- 風船モードの一連のシーケンス実行 ---
// target_idx: 1=A(Btn1), 2=B(Btn2), 3=C(Btn3)
void execute_sequence(int target_idx) {
  if (target_idx < 1 || target_idx > 3) return;
  
  Serial.print("Seq Start: "); Serial.println(target_idx);

  // 1. X軸移動 (位置A, B, C)
  stepperX.moveTo(POS_X[target_idx]);
  stepperX.runToPosition();
  delay(200);

  // 2. Y軸回転
  stepperY.move(Y_ACT_STEPS);
  stepperY.runToPosition();
  delay(200);

  // 3. 電磁石ON (押し付け)
  digitalWrite(PIN_SOLENOID, HIGH);
  delay(500);

  // 4. バルブ開 (180度 = 3.14 rad)
  actuator_move(3.14);

  // 5. 3秒待機
  delay(3000);

  // 6. バルブ閉 (戻す)
  actuator_move(0.0);

  // 7. 電磁石OFF
  digitalWrite(PIN_SOLENOID, LOW);
  delay(500);

  // 8. Y軸戻し
  stepperY.move(-Y_ACT_STEPS);
  stepperY.runToPosition();
  delay(200);

  // 9. X軸初期位置へ
  stepperX.moveTo(0);
  stepperX.runToPosition();
  delay(200);

  // 10. サーボアクション
  Servo *target_servo;
  if (target_idx == 1) target_servo = &s1;
  else if (target_idx == 2) target_servo = &s2;
  else target_servo = &s3;

  target_servo->write(30);  // 30度
  delay(1000);
  target_servo->write(0);   // 元に戻す (-30度動作相当として0へ)
  
  Serial.println("Seq Done.");
}

// タイムアウト付きでステッピングモーターを動かす関数
// timeout_ms: 制限時間 (ミリ秒)
// 戻り値: true=正常到達, false=タイムアウト中断
bool runToPositionWithTimeout(AccelStepper &stepper, unsigned long timeout_ms) {
  unsigned long start_time = millis();
  
  while (stepper.distanceToGo() != 0) {
    stepper.run();
    
    // タイムアウト判定
    if (millis() - start_time > timeout_ms) {
      Serial.println("Stepper Timeout! Force Stop.");
      // 現在位置を目標位置に設定しなおして即座に停止させる
      // (これをしないと、次に動かすときに急加速したり挙動がおかしくなるため)
      stepper.moveTo(stepper.currentPosition()); 
      return false; // 失敗
    }
  }
  return true; // 成功
}

// テスト用関数
// target_idx: 1, 2, 3
void execute_test_sequence(int target_idx) {
  if (target_idx < 1 || target_idx > 3) return;

  Serial.print("Test Seq (Full): "); Serial.println(target_idx);

  Servo *sv;
  if (target_idx == 1) sv = &s1;
  else if (target_idx == 2) sv = &s2;
  else sv = &s3;

  // 1. Y軸 前進
  Serial.println(" -> Y-Axis Forward");
  stepperY.move(2850); // 適切なステップ数に調整してください
  runToPositionWithTimeout(stepperY, 5000); 
  delay(200);

  // 2. 電磁石 吸着
  Serial.println(" -> Solenoid ON");
  digitalWrite(PIN_SOLENOID, HIGH);
  delay(1000); 

  // 3. バルブ動作
  Serial.println(" -> Valve OPEN");
  
  // これだけで「移動して、完了するまで待機」します
  actuator_move(0.3); 
  
  // 開いた状態で少しガスを入れる時間
  delay(1500); 

  Serial.println(" -> Valve CLOSE");
  
  // 0に戻る
  actuator_move(0.0); 

  Serial.println(" -> Valve Done");

  // 4. 電磁石 解放
  Serial.println(" -> Solenoid OFF");
  digitalWrite(PIN_SOLENOID, LOW);
  delay(500);

  // 5. Y軸 戻る
  Serial.println(" -> Y-Axis Return");
  stepperY.moveTo(0); 
  runToPositionWithTimeout(stepperY, 5000);
  delay(200);

  // 6. サーボ動作
  Serial.println(" -> Servo Action");
  sv->write(45);
  delay(1000);
  sv->write(0);
  delay(500);

  Serial.println("Test Done.");
}
