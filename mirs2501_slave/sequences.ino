#include "define.h"

// 外部参照
extern Servo s1, s2, s3;
extern AccelStepper stepperX, stepperY;
extern const long POS_X[]; // 本番用座標 (mainで定義されている想定)

void actuator_move(double target_rad);

// ★追加: テスト動作用のX座標定義 {HOME, BtnA, BtnB, BtnC}
const long TARGET_POS_X[] = {0, 1000, 2000, 3000}; 

// =========================================================
// タイムアウト付きでステッピングモーターを動かす関数
// timeout_ms: 制限時間 (ミリ秒)
// 戻り値: true=正常到達, false=タイムアウト中断
// =========================================================
bool runToPositionWithTimeout(AccelStepper &stepper, unsigned long timeout_ms) {
  unsigned long start_time = millis();
  
  while (stepper.distanceToGo() != 0) {
    stepper.run();
    
    // タイムアウト判定
    if (millis() - start_time > timeout_ms) {
      Serial.println("Stepper Timeout! Force Stop.");
      // 現在位置を目標位置に設定しなおして即座に停止させる
      stepper.moveTo(stepper.currentPosition()); 
      return false; // 失敗
    }
  }
  return true; // 成功
}

// =========================================================
// 風船モードの一連のシーケンス実行 (本番用)
// target_idx: 1=A(Btn1), 2=B(Btn2), 3=C(Btn3)
// =========================================================
void execute_sequence(int target_idx) {
  if (target_idx < 1 || target_idx > 3) return;
  
  Serial.print("Seq Start: "); Serial.println(target_idx);

  // 1. X軸移動 (位置A, B, C)
  stepperX.moveTo(POS_X[target_idx]);
  runToPositionWithTimeout(stepperX, 10000); // タイムアウト追加で安全化
  delay(200);

  // 2. Y軸回転
  stepperY.move(Y_ACT_STEPS);
  runToPositionWithTimeout(stepperY, 5000);
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
  runToPositionWithTimeout(stepperY, 5000);
  delay(200);

  // 9. X軸初期位置へ
  stepperX.moveTo(0);
  runToPositionWithTimeout(stepperX, 10000);
  delay(200);

  // 10. サーボアクション
  Servo *target_servo;
  if (target_idx == 1) target_servo = &s1;
  else if (target_idx == 2) target_servo = &s2;
  else target_servo = &s3;

  target_servo->write(30);  // 30度
  delay(1000);
  target_servo->write(0);   // 元に戻す
  
  Serial.println("Seq Done.");
}

// =========================================================
// テスト用シーケンス (X移動 -> 仕事 -> Y原点復帰 -> X原点復帰)
// target_idx: 1, 2, 3
// =========================================================
void execute_test_sequence(int target_idx) {
  // インデックスチェック
  if (target_idx < 1 || target_idx > 3) return;

  Serial.print("Test Seq (Full): "); Serial.println(target_idx);

  // サーボ選択
  Servo *sv;
  if (target_idx == 1) sv = &s1;
  else if (target_idx == 2) sv = &s2;
  else sv = &s3;

  // ---------------------------------------------------
  // 1. X軸 移動 (指定位置へ)
  // ---------------------------------------------------
  long target_x = TARGET_POS_X[target_idx];
  Serial.print(" -> X-Axis Move to: "); Serial.println(target_x);
  
  stepperX.moveTo(target_x);
  
  // タイムアウト付きで移動 (X軸は距離が長いので10秒設定)
  if (!runToPositionWithTimeout(stepperX, 10000)) {
    Serial.println("Error: X-Axis Move Timeout");
    return; // 移動に失敗したらここで中断
  }
  delay(200);

  // ---------------------------------------------------
  // 2. Y軸 & バルブ動作
  // ---------------------------------------------------
  
  // --- Y軸 前進 ---
  Serial.println(" -> Y-Axis Forward");
  stepperY.move(Y_ACT_STEPS); 
  runToPositionWithTimeout(stepperY, 5000); 
  delay(200);

  // --- 電磁石 吸着 ---
  Serial.println(" -> Solenoid ON");
  digitalWrite(PIN_SOLENOID, HIGH);
  delay(1000); 

  // --- バルブ動作 (開く) ---
  Serial.println(" -> Valve OPEN");
  actuator_move(0.79); 
  delay(2000); 

  // --- バルブ動作 (閉じる) ---
  Serial.println(" -> Valve CLOSE");
  actuator_move(0.0); 
  Serial.println(" -> Valve Done");

  // --- 電磁石 解放 ---
  Serial.println(" -> Solenoid OFF");
  digitalWrite(PIN_SOLENOID, LOW);
  delay(500);

  // --- Y軸 戻る (Homing) ---
  Serial.println(" -> Y-Axis Return (Homing)");
  stepperY.setSpeed(HOMING_SPEED_Y); // 戻る方向へ速度設定

  unsigned long start_time = millis();
  while (digitalRead(PIN_LIMIT_Y) == HIGH) {
    stepperY.runSpeed();
    // ★修正: IMEOUT -> TIMEOUT
    if (millis() - start_time > TIMEOUT_Y_HOME) {
      Serial.println("Error: Y Limit timeout!");
      break;
    }
  }
  stepperY.setSpeed(0);
  stepperY.setCurrentPosition(0);
  stepperY.moveTo(0);
  delay(200);

  // ---------------------------------------------------
  // 3. X軸 戻る (Homing)
  // ---------------------------------------------------
  Serial.println(" -> X-Axis Return (Homing)");
  
  // 原点方向へ一定速度で移動
  stepperX.setSpeed(HOMING_SPEED_X); 

  start_time = millis();
  
  // X軸のリミットスイッチが押されるまでループ
  while (digitalRead(PIN_LIMIT_X) == HIGH) {
    stepperX.runSpeed();
    
    // X軸は長いのでタイムアウトを長めに
    if (millis() - start_time > TIMEOUT_X_HOME) {
      Serial.println("Error: X Limit timeout!");
      break;
    }
  }

  // 停止処理
  stepperX.setSpeed(0);
  stepperX.setCurrentPosition(0); // ここを新たなX原点とする
  stepperX.moveTo(0); // 念のため位置ズレ防止
  delay(200);

  // ---------------------------------------------------
  // 4. サーボ動作 (完了アクション)
  // ---------------------------------------------------
  Serial.println(" -> Servo Action");
  sv->write(45);
  delay(1000);
  sv->write(0);
  delay(500);

  Serial.println("Test Done.");
}
