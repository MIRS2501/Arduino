/*
 * ======================================================================================
 * プロジェクト名 : MIRS2501 BABLOON
 * ファイル名     : mg5_slave.ino
 * バージョン     : Ver 1.03
 * 最終更新日     : 2026/1/5
 * --------------------------------------------------------------------------------------
 * [ ピン配置 (Pin Assignment) - Arduino Uno (I2C Addr: 8) ]
 * 0 (D0) : Serial RX (デバッグ用)
 * 1 (D1) : Serial TX (デバッグ用)
 * 2 (D2) : ステッピング X軸 STEP
 * 3 (D3) : ステッピング Y軸 STEP
 * 4 (D4) : 電磁石 (ソレノイド) 制御
 * 5 (D5) : 新規アクチュエータ PWM
 * 6 (D6) : ステッピング X軸 DIR
 * 7 (D7) : ステッピング Y軸 DIR
 * 8 (D8) : 新規アクチュエータ DIR
 * 9 (D9) : ステッピング X軸 リミットスイッチ
 * 10 (D10): ステッピング Y軸 リミットスイッチ
 * 11 (D11): サーボモータ 1 (風船1用)
 * 12 (D12): サーボモータ 2 (風船2用)
 * 13 (D13): サーボモータ 3 (風船3用)
 * A0 (14) : 新規アクチュエータ エンコーダ A相 (PCINT)
 * A1 (15) : 新規アクチュエータ エンコーダ B相 (PCINT)
 * A2 (16) : 追加ボタン A (巡回開始)
 * A3 (17) : 追加ボタン B (風船モード移行)
 * A4 (18) : I2C SDA (Master通信)
 * A5 (19) : I2C SCL (Master通信)
 * ======================================================================================
 */

#include <Wire.h>
#include <Servo.h>
#include <AccelStepper.h>
#include "define.h"

// --- オブジェクト ---
Servo s1, s2, s3;
AccelStepper stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);

// --- 定数 ---
const long POS_X[] = {0, 1000, 2000, 3000}; // 初期, A, B, C
//const long Y_ACT_STEPS = 500; // Y回転量

// 状態変数
volatile int received_cmd = 0;
volatile bool is_busy = false; // 動作中フラグ

void setup() {
  Serial.begin(115200);

  // ピン初期化
  pinMode(PIN_SOLENOID, OUTPUT);
  pinMode(PIN_BTN_A, INPUT_PULLUP);
  pinMode(PIN_BTN_B, INPUT_PULLUP);

  // モータ類初期化
  s1.attach(PIN_SERVO_1); s1.write(0);
  s2.attach(PIN_SERVO_2); s2.write(0);
  s3.attach(PIN_SERVO_3); s3.write(0);
  
  stepperX.setMaxSpeed(1000); stepperX.setAcceleration(500);
  stepperY.setMaxSpeed(1000); stepperY.setAcceleration(500);
  stepperX.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);

  // アクチュエータ (actuator.ino)
  actuator_open(); 

  // I2C設定 (アドレス8)
  Wire.begin(8);
  Wire.onReceive(receiveEvent); // 受信 (Master -> Slave)
  Wire.onRequest(requestEvent); // 送信 (Master <- Slave)
  
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

// --- I2C 送信 (ボタン状態とBusyフラグを返す) ---
// Masterがデータを要求した時に呼ばれる
void requestEvent() {
  // ビットパックして送信
  // bit0: Button A (0=Pressed, 1=Released) ※プルアップなので反転して送る
  // bit1: Button B
  // bit7: Busy Flag
  
  byte status = 0;
  if (digitalRead(PIN_BTN_A) == LOW) status |= (1 << 0);
  if (digitalRead(PIN_BTN_B) == LOW) status |= (1 << 1);
  if (is_busy) status |= (1 << 7);
  
  Wire.write(status);
}
