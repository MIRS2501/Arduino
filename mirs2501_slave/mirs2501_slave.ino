/*
 * ======================================================================================
 * プロジェクト名 : MIRS2501 BABLOON
 * ファイル名     : mirs2501_slave.ino
 * ======================================================================================
 */

#include <Wire.h>
#include <Servo.h>
#include <AccelStepper.h>
#include "define.h"

// 変更前
// bool home_axis(..., unsigned long timeout_ms);

// 変更後
bool home_axis(AccelStepper &stepper, int limit_pin, long speed, int backoff_dir);
void execute_sequence(int target_idx);
void execute_test_sequence(int target_idx);

// --- オブジェクト ---
Servo s1, s2, s3;
AccelStepper stepperX(AccelStepper::DRIVER, PIN_STEP_X, PIN_DIR_X);
AccelStepper stepperY(AccelStepper::DRIVER, PIN_STEP_Y, PIN_DIR_Y);

// --- 定数 ---
const long POS_X[] = {0, 1000, 2000, 3000}; // 初期, A, B, C

// 状態変数
volatile int received_cmd = 0;
volatile bool is_busy = false; // 動作中フラグ

void setup() {
  Serial.begin(115200);

  // ピン初期化
  pinMode(PIN_SOLENOID, OUTPUT);
  pinMode(PIN_BTN_A, INPUT_PULLUP);
  pinMode(PIN_BTN_B, INPUT_PULLUP);
  
  // リミットスイッチの設定 (プルアップ)
  pinMode(PIN_LIMIT_X, INPUT_PULLUP);
  pinMode(PIN_LIMIT_Y, INPUT_PULLUP);

  // モータ類初期化
  s1.attach(PIN_SERVO_1); s1.write(0);
  s2.attach(PIN_SERVO_2); s2.write(0);
  s3.attach(PIN_SERVO_3); s3.write(0);
  
  stepperX.setMaxSpeed(10000); stepperX.setAcceleration(5000);
  stepperY.setMaxSpeed(1000);
  stepperY.setAcceleration(500);

  // Y軸の回転方向(DIRピン)反転設定
  stepperY.setPinsInverted(true, false, false);
  
  stepperX.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);

  // アクチュエータ (actuator.ino)
  actuator_open();

  // I2C設定 (アドレス8)
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  // ★★★ 起動時の原点復帰 ★★★
  Serial.println("Start Homing...");
  
  // Y軸: さっき成功したなら「1」のままでOK
  home_axis(stepperY, PIN_LIMIT_Y, HOMING_SPEED_Y, -1); 
  
  // X軸: さっき逆に動いたので、「-1」に変更！
  home_axis(stepperX, PIN_LIMIT_X, HOMING_SPEED_X, 1);
  
  Serial.println("Slave Ready.");
}

void loop() {
  if (received_cmd != 0) {
    is_busy = true;

    // コマンド番号で分岐
    if (received_cmd >= 1 && received_cmd <= 3) {
      // 通常の風船シーケンス
      execute_sequence(received_cmd);
    } else if (received_cmd >= 11 && received_cmd <= 13) {
      // サーボ単体テスト
      execute_test_sequence(received_cmd - 10);
    }
    
    received_cmd = 0;
    is_busy = false;
  }
}

// --- I2C 受信 (コマンド受け取り) ---
void receiveEvent(int howMany) {
  received_cmd = Wire.read();
}

// --- I2C 送信 (ステータス返し) ---
void requestEvent() {
  byte status = 0;
  // ボタン状態 (押されるとLOWなので反転して1にする)
  if (digitalRead(PIN_BTN_A) == LOW) status |= (1 << 0);
  if (digitalRead(PIN_BTN_B) == LOW) status |= (1 << 1);
  if (is_busy) status |= (1 << 7);
  
  Wire.write(status);
}
