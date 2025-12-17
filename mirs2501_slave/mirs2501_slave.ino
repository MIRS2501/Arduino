#include <Wire.h>
#include <Servo.h>
#include <AccelStepper.h>

// --- ピン定義 (スレーブ側は自由に使える) ---
#define PIN_SOLENOID 4
#define PIN_SERVO_1  11
#define PIN_SERVO_2  12
#define PIN_SERVO_3  13

// ステッピング (元のコードのピンを使用)
#define X_STEP_PIN 2
#define X_DIR_PIN  5
#define Y_STEP_PIN 3
#define Y_DIR_PIN  6
#define X_LIMIT_PIN 9 
#define Y_LIMIT_PIN 10

// --- オブジェクト生成 ---
Servo s1, s2, s3;
AccelStepper stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);

// --- 変数 ---
volatile int received_command = 0; // 受信したコマンド
const long POSITIONS[] = {0, 1000, 2000, 3000}; 

// --- セットアップ ---
void setup() {
  Serial.begin(115200);
  
  // 電磁石
  pinMode(PIN_SOLENOID, OUTPUT);
  digitalWrite(PIN_SOLENOID, LOW);

  // サーボ
  s1.attach(PIN_SERVO_1); s1.write(0);
  s2.attach(PIN_SERVO_2); s2.write(0);
  s3.attach(PIN_SERVO_3); s3.write(0);

  // ステッピング設定
  stepperX.setMaxSpeed(1000);
  stepperX.setAcceleration(500);
  stepperY.setMaxSpeed(1000);
  stepperY.setAcceleration(500);
  
  // I2Cスレーブ設定 (アドレス8)
  Wire.begin(8); 
  Wire.onReceive(receiveEvent); // 受信時の割り込み関数登録

  Serial.println("Slave Ready.");
}

// --- メインループ ---
void loop() {
  // コマンドが来ていたら実行する
  if (received_command != 0) {
    execute_command(received_command);
    received_command = 0; // 処理完了
  }
}

// --- I2C受信割り込み ---
void receiveEvent(int howMany) {
  while (Wire.available()) {
    received_command = Wire.read(); // コマンドを読み取って保持
  }
}

// --- コマンド実行処理 ---
void execute_command(int cmd) {
  Serial.print("Exec Cmd: "); Serial.println(cmd);

  switch (cmd) {
    // [1-9] サーボ制御
    case 1: servo_action(s1); break;
    case 2: servo_action(s2); break;
    case 3: servo_action(s3); break;

    // [10-19] 電磁石制御
    case 10: digitalWrite(PIN_SOLENOID, HIGH); break; // 吸着
    case 11: digitalWrite(PIN_SOLENOID, LOW);  break; // 開放

    // [20-29] ステッピング制御
    case 20: stepper_sequence(POSITIONS[1]); break; // Pos A
    case 21: stepper_sequence(POSITIONS[2]); break; // Pos B
    case 22: stepper_sequence(POSITIONS[3]); break; // Pos C
    
    default: break;
  }
}

// --- 個別のアクション関数 ---

void servo_action(Servo &s) {
  s.write(30);
  delay(1000);
  s.write(0);
}

void stepper_sequence(long target_x) {
  // X移動
  stepperX.moveTo(target_x);
  stepperX.runToPosition(); // ブロック動作

  // Y回転
  stepperY.move(500);
  stepperY.runToPosition();

  delay(1000);

  // Y戻し
  stepperY.move(-500);
  stepperY.runToPosition();

  // X戻し
  stepperX.moveTo(0);
  stepperX.runToPosition();
}
