#include "AngleController.h"
#include "define.h"

// ==========================================
// 設定エリア
// ==========================================
// ★重要: モーターが暴走する場合、ここを true ⇔ false 切り替えてください
const bool REVERSE_DIR = true; 

// PIDゲインと制御パラメータ
// Kp: 反応の強さ
// Ki: ズレを修正する力
// Kd: ブレーキの力
// MinPWM: モーターが回り始める最低パワー (不感帯補償)
// IntegralLimit: 積分の暴走を防ぐリミット値
const double  VAL_KP = 100.0;
const double  VAL_KI = 5.0;
const double  VAL_KD = 0.0;
const int     VAL_MIN_PWM = 35;
const double  VAL_I_LIMIT = 20.0;

// ==========================================

AngleController actuator(ACT_PPR, ACT_GEAR_RATIO, ACT_ENC_MAG, 10);
volatile long act_enc_count = 0;

// --- ハードウェア関数 ---
long act_read_encoder() {
  noInterrupts();
  long count = act_enc_count;
  interrupts();
  return count;
}

void act_reset_encoder() {
  noInterrupts();
  act_enc_count = 0;
  interrupts();
}

// モーター出力 (回転方向反転ロジック付き)
void act_set_motor(int pwm) {
  int output_pwm = pwm;
  
  // フラグによる方向制御
  // REVERSE_DIR が true なら HIGH/LOW を逆転させる
  bool dir_signal = (output_pwm > 0) ? LOW : HIGH; 
  if (REVERSE_DIR) dir_signal = !dir_signal;

  digitalWrite(PIN_ACT_DIR, dir_signal ? HIGH : LOW);
  analogWrite(PIN_ACT_PWM, abs(output_pwm));
}

// ==========================================
// 割り込み処理 (A0, A1ピン用)
// ==========================================
ISR(PCINT1_vect) {
  static boolean A_prev = false;
  static boolean B_prev = false;

  // A0(PC0) と A1(PC1) を読み取る
  // ★重要: ビット演算結果を HIGH/LOW に正規化
  boolean A_curr = (PINC & (1 << PC0)) ? HIGH : LOW; 
  boolean B_curr = (PINC & (1 << PC1)) ? HIGH : LOW; 

  if (A_curr != A_prev || B_curr != B_prev) {
    if (A_prev == LOW && A_curr == HIGH) { // A立ち上がり
      (B_curr == LOW) ? act_enc_count++ : act_enc_count--;
    } 
    else if (A_prev == HIGH && A_curr == LOW) { // A立ち下がり
      (B_curr == HIGH) ? act_enc_count++ : act_enc_count--;
    } 
    else if (B_prev == LOW && B_curr == HIGH) { // B立ち上がり
      (A_curr == HIGH) ? act_enc_count++ : act_enc_count--;
    } 
    else if (B_prev == HIGH && B_curr == LOW) { // B立ち下がり
      (A_curr == LOW) ? act_enc_count++ : act_enc_count--;
    }
  }
  
  A_prev = A_curr;
  B_prev = B_curr;
}


// ==========================================
// 公開関数
// ==========================================
void actuator_open() {
  // ピン設定
  pinMode(PIN_ACT_PWM, OUTPUT);
  pinMode(PIN_ACT_DIR, OUTPUT);
  pinMode(PIN_ACT_ENC_A, INPUT_PULLUP); // A0
  pinMode(PIN_ACT_ENC_B, INPUT_PULLUP); // A1

  actuator.attachHardware(act_set_motor, act_read_encoder, act_reset_encoder);
  
  // 制御パラメータ設定 (AngleController大幅改変版に対応)
  actuator.setParams(VAL_KP, VAL_KI, VAL_KD, VAL_MIN_PWM, VAL_I_LIMIT);

  // --- 割り込み設定 (A0, A1) ---
  noInterrupts();
  PCICR  |= (1 << PCIE1);    // PCINT1グループ有効化
  PCMSK1 |= (1 << PCINT8);   // A0ピン有効化
  PCMSK1 |= (1 << PCINT9);   // A1ピン有効化
  
  // (念のため他のピンの割り込みは切っておく)
  PCMSK1 &= ~(1 << PCINT10); // A2 無効
  PCMSK1 &= ~(1 << PCINT11); // A3 無効
  interrupts();

  act_reset_encoder();
}

void actuator_move(double target_angle_rad) {
  Serial.print("Actuator Move: ");
  Serial.println(target_angle_rad);
  
  actuator.moveToAngle(target_angle_rad);
  
  Serial.println("Actuator Done.");
}
/*#include "AngleController.h"
#include "define.h"

// --- グローバル変数 ---
// クラスのインスタンス生成 (ループ周期 5ms)
AngleController actuator(ACT_PPR, ACT_GEAR_RATIO, ACT_ENC_MAG, 5);

// エンコーダカウント用 (割り込み内で操作するため volatile)
volatile long act_enc_count = 0;


// ==========================================
// ハードウェア依存関数 (クラスに渡す関数)
// ==========================================

// 1. エンコーダカウント取得
long act_read_encoder() {
  noInterrupts(); // 読み取り中に割り込みが来ないようにする
  long count = act_enc_count;
  interrupts();
  return count;
}

// 2. エンコーダリセット
void act_reset_encoder() {
  noInterrupts();
  act_enc_count = 0;
  interrupts();
}

// 3. モーター出力 (-255 ～ 255)
void act_set_motor(int pwm) {
  if (pwm > 0) {
    digitalWrite(PIN_ACT_DIR, LOW); // 正転
    analogWrite(PIN_ACT_PWM, pwm);
  } else if (pwm < 0) {
    digitalWrite(PIN_ACT_DIR, HIGH);  // 逆転
    analogWrite(PIN_ACT_PWM, -pwm);  // 絶対値を出力
  } else {
    // 停止 (ブレーキ)
    digitalWrite(PIN_ACT_DIR, LOW);
    analogWrite(PIN_ACT_PWM, 0);
  }
}

// ==========================================
// 割り込み処理 (PCINT: Pin Change Interrupt)
// Unoのピン2,3が埋まっているため、A0,A1で割り込みを行う特殊処理
// ==========================================
ISR(PCINT1_vect) {
  static boolean A_prev = false;
  static boolean B_prev = false;

  //boolean A_curr = (PINC & (1 << PC0)); // A0の状態読み取り
  //boolean B_curr = (PINC & (1 << PC1)); // A1の状態読み取り
  // 【修正点】ビット演算の結果を 0 か 1 に正規化する (三項演算子を使用)
  boolean A_curr = (PINC & (1 << PC0)) ? HIGH : LOW;
  boolean B_curr = (PINC & (1 << PC1)) ? HIGH : LOW;

  // 変化があった場合のみカウント処理 (4逓倍ロジック)
  if (A_curr != A_prev || B_curr != B_prev) {
    if (A_prev == LOW && A_curr == HIGH) { // A立ち上がり
      (B_curr == LOW) ? act_enc_count++ : act_enc_count--;
    } 
    else if (A_prev == HIGH && A_curr == LOW) { // A立ち下がり
      (B_curr == HIGH) ? act_enc_count++ : act_enc_count--;
    } 
    else if (B_prev == LOW && B_curr == HIGH) { // B立ち上がり
      (A_curr == HIGH) ? act_enc_count++ : act_enc_count--;
    } 
    else if (B_prev == HIGH && B_curr == LOW) { // B立ち下がり
      (A_curr == LOW) ? act_enc_count++ : act_enc_count--;
    }
  }
  
  A_prev = A_curr;
  B_prev = B_curr;
}


// ==========================================
// 公開関数
// ==========================================

// セットアップ (setup()内で呼ぶ)
void actuator_open() {
  // ピン設定
  pinMode(PIN_ACT_PWM, OUTPUT);
  pinMode(PIN_ACT_DIR, OUTPUT);
  pinMode(PIN_ACT_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ACT_ENC_B, INPUT_PULLUP);

  // ハードウェア関数をクラスに登録
  actuator.attachHardware(act_set_motor, act_read_encoder, act_reset_encoder);
  
  // PIDゲイン設定 (必要に応じて調整)
  actuator.setGains(150.0, 0.10, 50.0);

  // --- ピン変化割り込み(PCINT)の設定 (A0, A1ピン用) ---
  noInterrupts();
  PCICR  |= (1 << PCIE1);    // PCINT1グループ(A0-A5)の割り込み有効化
  PCMSK1 |= (1 << PCINT8);   // A0ピン (PCINT8) を対象にする
  PCMSK1 |= (1 << PCINT9);   // A1ピン (PCINT9) を対象にする
  interrupts();

  act_reset_encoder();
}

// 指定角度まで動かす関数 [単位: rad]
// ※ 180度 = 3.14 rad
void actuator_move(double target_angle_rad) {
  Serial.print("Actuator Move: ");
  Serial.println(target_angle_rad);
  
  // クラスの関数を呼び出し (目標位置に行くまで処理がブロックされます)
  actuator.moveToAngle(target_angle_rad);
  
  Serial.println("Actuator Done.");
}*/
