/*
 * ======================================================================================
 * プロジェクト名 : MIRS2501 BABLOON
 * ファイル名     : mirs2501_bubble.ino
 * バージョン     : Ver 1.00
 * 最終更新日     : 2025/12/19
 * --------------------------------------------------------------------------------------
 * [ ピン配置 (Pin Assignment) - Arduino Uno ]
 * 2 (D2) : リミットスイッチ
 * 3 (D3) : モーター速度
 * 4 (D4) : モーター回転方向
 * ======================================================================================
 */

// --- ピン定義 ---
const int PIN_SWITCH = 2; // リミットスイッチ (D2とGNDに接続)
const int PIN_PWM    = 3; // モーター速度 (PWM)
const int PIN_DIR    = 4; // モーター回転方向

void setup() {
  // スイッチ設定 (内部プルアップ有効：押すとLOWになります)
  pinMode(PIN_SWITCH, INPUT_PULLUP);

  // モーター設定
  pinMode(PIN_PWM, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);

  // 最初は停止
  analogWrite(PIN_PWM, 0);
}

void loop() {
  // スイッチが押されたら (LOWになったら)
  if (digitalRead(PIN_SWITCH) == LOW) {
    
    // 1. 回転方向を決める (HIGHまたはLOW)
    digitalWrite(PIN_DIR, LOW); 

    // 2. モーターを回す (PWM 100)
    analogWrite(PIN_PWM, 200);

    // 3. 3秒待つ
    delay(5000);

    // 4. 停止する
    analogWrite(PIN_PWM, 0);

    // チャタリング(誤検知)防止のため少し待つ
    delay(500);
  }
}
