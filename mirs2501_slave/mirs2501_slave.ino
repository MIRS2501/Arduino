/*
 * ======================================================================================
 * プロジェクト名 : MIRS2501 BABLOON
 * ファイル名     : mg5_slave.ino
 * バージョン     : Ver 1.00
 * 最終更新日     : 2025/12/18
 * --------------------------------------------------------------------------------------
 * [ ピン配置 (Pin Assignment) - Arduino Uno (I2C Addr: 8) ]
 * 0 (D0) : Serial RX (デバッグ用)
 * 1 (D1) : Serial TX (デバッグ用)
 * 2 (D2) : ステッピング X軸 STEP
 * 3 (D3) : ステッピング Y軸 STEP
 * 4 (D4) : 電磁石 (ソレノイド) 制御
 * 5 (D5) : ステッピング X軸 DIR
 * 6 (D6) : ステッピング Y軸 DIR
 * 7 (D7) : 新規アクチュエータ PWM
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

// --- ピン定義 ---
#define PIN_SOLENOID 4
#define PIN_SERVO_1  11
#define PIN_SERVO_2  12
#define PIN_SERVO_3  13
// ステッピング (以前と同じ)
#define X_STEP_PIN 2
#define X_DIR_PIN  5
#define Y_STEP_PIN 3
#define Y_DIR_PIN  6
#define X_LIMIT_PIN 9 
#define Y_LIMIT_PIN 10
// 追加ボタン
#define PIN_BTN_A    A2 // 巡回開始
#define PIN_BTN_B    A3 // 風船モード移行

// --- オブジェクト ---
Servo s1, s2, s3;
AccelStepper stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);

// --- 定数 ---
const long POS_X[] = {0, 1000, 2000, 3000}; // 初期, A, B, C
const long Y_ACT_STEPS = 500; // Y回転量

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
  // コマンド処理
  if (received_cmd != 0) {
    is_busy = true;
    execute_sequence(received_cmd);
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

// --- 風船モードの一連のシーケンス実行 ---
// target_idx: 1=A(Btn1), 2=B(Btn2), 3=C(Btn3)
void execute_sequence(int target_idx) {
  if (target_idx < 1 || target_idx > 3) return;
  
  Serial.print("Seq Start: "); Serial.println(target_idx);

  // 1. X軸移動 (位置A, B, C)
  stepperX.moveTo(POS_X[target_idx]);
  stepperX.runToPosition();

  // 2. Y軸回転
  stepperY.move(Y_ACT_STEPS);
  stepperY.runToPosition();

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

  // 9. X軸初期位置へ
  stepperX.moveTo(0);
  stepperX.runToPosition();

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
