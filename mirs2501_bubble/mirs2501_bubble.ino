/*
 * ======================================================================================
 * プロジェクト名 : MIRS2501 BABLOON
 * ファイル名     : mirs2501_bubble.ino
 * バージョン     : Ver 1.00
 * 最終更新日     : 2025/12/19
 /*
 * ======================================================================================
 * プロジェクト名 : MIRS2501 BABLOON
 * ファイル名     : mirs2501_bubble_v2.ino
 * バージョン     : Ver 1.10 (Dual Motor)
 * 最終更新日     : 2026/01/24
 * --------------------------------------------------------------------------------------
 * [ ピン配置 (Pin Assignment) - Arduino Uno ]
 * 2 (D2) : リミットスイッチ
 * 3 (D3) : モーター1 速度 (PWM)
 * 4 (D4) : モーター1 回転方向
 * 5 (D5) : モーター2 速度 (PWM)  <-- 追加
 * 6 (D6) : モーター2 回転方向    <-- 追加
 * ======================================================================================
 */

// --- ピン定義 ---
const int PIN_SWITCH = 2; // リミットスイッチ (D2とGNDに接続)

// モーター1 (既存)
const int PIN_PWM1   = 3; // モーター1 速度 (PWM)
const int PIN_DIR1   = 4; // モーター1 回転方向

// モーター2 (新規追加)
const int PIN_PWM2   = 5; // モーター2 速度 (PWM) ※D5はPWM対応ピン
const int PIN_DIR2   = 6; // モーター2 回転方向

void setup() {
  // スイッチ設定 (内部プルアップ有効：押すとLOWになります)
  pinMode(PIN_SWITCH, INPUT_PULLUP);

  // モーター1設定
  pinMode(PIN_PWM1, OUTPUT);
  pinMode(PIN_DIR1, OUTPUT);

  // モーター2設定
  pinMode(PIN_PWM2, OUTPUT);
  pinMode(PIN_DIR2, OUTPUT);

  // 最初は両方停止
  analogWrite(PIN_PWM1, 0);
  analogWrite(PIN_PWM2, 0);
}

void loop() {
  // スイッチが押されたら (LOWになったら)
  if (digitalRead(PIN_SWITCH) == LOW) {
    
    // 1. 回転方向を決める (HIGHまたはLOW)
    // ※モーターの配線によっては、片方をHIGH、もう片方をLOWにする必要があるかもしれません
    digitalWrite(PIN_DIR1, LOW); 
    digitalWrite(PIN_DIR2, LOW); 

    // 2. モーターを回す (PWM 70)
    // 2つのモーターを同時に駆動します
    analogWrite(PIN_PWM1, 70);
    analogWrite(PIN_PWM2, 70);

    // 3. 5秒待つ (コメントは3秒ですがコードは5000ms=5秒になっています)
    delay(5000);

    // 4. 停止する
    analogWrite(PIN_PWM1, 0);
    analogWrite(PIN_PWM2, 0);

    // チャタリング(誤検知)防止のため少し待つ
    delay(500);
  }
}
