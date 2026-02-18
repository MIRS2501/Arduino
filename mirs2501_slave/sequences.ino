#include "define.h"

// 外部参照
extern Servo s1, s2, s3;
extern AccelStepper stepperX, stepperY;
extern const long POS_X[]; 

// 外部関数宣言
void actuator_move(double target_rad);
const long TARGET_POS_X[] = {0, 49200, 25100, 300};

// =========================================================
// [汎用] 原点復帰関数 (方向指定マニュアル版)
// backoff_dir: 1 ならプラス方向にバック、-1 ならマイナス方向にバック
// =========================================================
bool home_axis(AccelStepper &stepper, int limit_pin, long speed, int backoff_dir) {
  unsigned long start_time = millis();
  unsigned long timeout_ms = 60000; // 共通タイムアウト

  Serial.print("  [Homing] Start. Pin: "); Serial.println(limit_pin);

  // --------------------------------------------------------
  // 0. 安全確認: 開始時にすでにスイッチが押されている場合
  // --------------------------------------------------------
  if (digitalRead(limit_pin) == LOW) {
    Serial.println("  [Homing] Switch pressed at start. Backing off...");
    // 引数で指定された「バック方向」に動く
    stepper.move(backoff_dir * 500); 
    stepper.runToPosition();
    delay(500); 
    stepper.setCurrentPosition(0);
  }

  // --------------------------------------------------------
  // 1. 原点探索 (Phase 1)
  // --------------------------------------------------------
  stepper.setSpeed(speed);
  start_time = millis();

  while (digitalRead(limit_pin) == HIGH) { 
    stepper.runSpeed();
    if (millis() - start_time > timeout_ms) {
      Serial.println("  [Homing] Timeout!");
      stepper.stop();
      return false;
    }
  }
  
  // スイッチ検知 -> 停止
  stepper.stop(); 
  Serial.println("  [Homing] Switch HIT!");
  delay(500); 

  // --------------------------------------------------------
  // 2. バックオフ動作 (Phase 2)
  // --------------------------------------------------------
  Serial.print("  [Homing] Backing off. Dir: "); Serial.println(backoff_dir);

  long backoff_steps = 200; // 戻る量
  
  // 指定された方向 (1 or -1) に動く
  stepper.move(backoff_dir * backoff_steps);
  stepper.runToPosition(); // 動くまで待つ

  // --------------------------------------------------------
  // 3. 完了
  // --------------------------------------------------------
  delay(200);
  Serial.println("  [Homing] Complete. Set Zero.");
  
  stepper.setCurrentPosition(0); 
  stepper.moveTo(0);             
  
  return true;
}

// =========================================================
// タイムアウト付き移動関数
// =========================================================
bool runToPositionWithTimeout(AccelStepper &stepper, unsigned long timeout_ms) {
  unsigned long start_time = millis();
  while (stepper.distanceToGo() != 0) {
    stepper.run();
    if (millis() - start_time > timeout_ms) {
      Serial.println("Timeout! Force Stop.");
      stepper.moveTo(stepper.currentPosition()); 
      return false;
    }
  }
  return true;
}

// =========================================================
// 本番用シーケンス
// =========================================================
void execute_sequence(int target_idx) {
  if (target_idx < 1 || target_idx > 3) return;
  Serial.print("Seq Start: "); Serial.println(target_idx);

  stepperX.moveTo(POS_X[target_idx]);
  runToPositionWithTimeout(stepperX, 10000);
  delay(200);

  stepperY.move(Y_ACT_STEPS);
  runToPositionWithTimeout(stepperY, 5000);
  delay(200);

  digitalWrite(PIN_SOLENOID, HIGH);
  delay(500);
  actuator_move(3.14);
  delay(3000);
  actuator_move(0.0);
  digitalWrite(PIN_SOLENOID, LOW);
  delay(500);

  stepperY.move(-Y_ACT_STEPS);
  runToPositionWithTimeout(stepperY, 5000);
  delay(200);

  stepperX.moveTo(0);
  runToPositionWithTimeout(stepperX, 10000);
  delay(200);

  Servo *target_servo;
  if (target_idx == 1) target_servo = &s1;
  else if (target_idx == 2) target_servo = &s2;
  else target_servo = &s3;

  target_servo->write(30); delay(1000); target_servo->write(0);
  
  Serial.println("Seq Done.");
}

// =========================================================
// テスト用シーケンス
// =========================================================
void execute_test_sequence(int target_idx) {
  if (target_idx < 1 || target_idx > 3) return;
  Serial.print("Test Seq: "); Serial.println(target_idx);

  Servo *sv;
  if (target_idx == 1) sv = &s1;
  else if (target_idx == 2) sv = &s2;
  else sv = &s3;

  stepperX.moveTo(TARGET_POS_X[target_idx]);
  runToPositionWithTimeout(stepperX, 60000);
  delay(200);

  stepperY.move(Y_ACT_STEPS); 
  runToPositionWithTimeout(stepperY, 5000); 
  delay(200);
  
  digitalWrite(PIN_SOLENOID, HIGH);
  delay(500);
  actuator_move(1);
  delay(800);
  actuator_move(0.0);
  digitalWrite(PIN_SOLENOID, LOW);
  delay(500);

  
  // 原点復帰 (ここもマニュアル指定が必要ですが、まずは起動時Setupを優先調整してください)
  // 仮設定: Y=1, X=-1
  Serial.println(" -> Y Homing...");
  home_axis(stepperY, PIN_LIMIT_Y, HOMING_SPEED_Y, -1); 
  delay(200);
  
  sv->write(45); delay(1000); sv->write(0); delay(500);
  
  Serial.println(" -> X Homing...");
  home_axis(stepperX, PIN_LIMIT_X, HOMING_SPEED_X, 1);
  delay(200);

  
  Serial.println("Test Done.");
}
