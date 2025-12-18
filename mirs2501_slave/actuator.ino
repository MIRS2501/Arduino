#include "AngleController.h"

// --- ピン定義 (空いているピンを使用) ---
// モーター駆動用
#define ACT_PIN_PWM  7   // PWMピン (enable)
#define ACT_PIN_DIR  8   // 回転方向

// エンコーダ用 (アナログピンをデジタル入力として使用)
#define ACT_PIN_ENC_A A0 // PCINT8
#define ACT_PIN_ENC_B A1 // PCINT9

// --- パラメータ設定 (使用するモータに合わせて変更してください) ---
#define ACT_PPR        12.0   // エンコーダのパルス数(1回転あたり)
#define ACT_GEAR_RATIO 50.0   // ギア比 (例: 50:1 なら 50.0)
#define ACT_ENC_MAG    4.0    // エンコーダ逓倍 (通常4逓倍)

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
    digitalWrite(ACT_PIN_DIR, HIGH); // 正転
    analogWrite(ACT_PIN_PWM, pwm);
  } else if (pwm < 0) {
    digitalWrite(ACT_PIN_DIR, LOW);  // 逆転
    analogWrite(ACT_PIN_PWM, -pwm);  // 絶対値を出力
  } else {
    // 停止 (ブレーキ)
    digitalWrite(ACT_PIN_DIR, LOW);
    analogWrite(ACT_PIN_PWM, 0);
  }
}

// ==========================================
// 割り込み処理 (PCINT: Pin Change Interrupt)
// Unoのピン2,3が埋まっているため、A0,A1で割り込みを行う特殊処理
// ==========================================
ISR(PCINT1_vect) {
  static boolean A_prev = false;
  static boolean B_prev = false;

  boolean A_curr = (PINC & (1 << PC0)); // A0の状態読み取り
  boolean B_curr = (PINC & (1 << PC1)); // A1の状態読み取り

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
  pinMode(ACT_PIN_PWM, OUTPUT);
  pinMode(ACT_PIN_DIR, OUTPUT);
  pinMode(ACT_PIN_ENC_A, INPUT_PULLUP);
  pinMode(ACT_PIN_ENC_B, INPUT_PULLUP);

  // ハードウェア関数をクラスに登録
  actuator.attachHardware(act_set_motor, act_read_encoder, act_reset_encoder);
  
  // PIDゲイン設定 (必要に応じて調整)
  // actuator.setGains(10.0, 0.1, 0.0);

  // --- ピン変化割り込み(PCINT)の設定 (A0, A1ピン用) ---
  noInterrupts();
  PCICR  |= (1 << PCIE1);    // PCINT1グループ(A0-A5)の割り込み有効化
  PCMSK1 |= (1 << PCINT8);   // A0ピン (PCINT8) を対象にする
  PCMSK1 |= (1 << PCINT9);   // A1ピン (PCINT9) を対象にする
  interrupts();
}

// 指定角度まで動かす関数 [単位: rad]
// ※ 180度 = 3.14 rad
void actuator_move(double target_angle_rad) {
  Serial.print("Actuator Move: ");
  Serial.println(target_angle_rad);
  
  // クラスの関数を呼び出し (目標位置に行くまで処理がブロックされます)
  actuator.moveToAngle(target_angle_rad);
  
  Serial.println("Actuator Done.");
}
