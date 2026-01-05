/*
 * ======================================================================================
 * プロジェクト名 : MIRS2501 BABLOON
 * ファイル名     : mirs2501_master.ino
 * バージョン     : Ver 1.02
 * 最終更新日     : 2025/1/4
 * --------------------------------------------------------------------------------------
 * [ ピン配置 (Pin Assignment) - Arduino Uno ]
 * 0 (D0) : Serial RX (PC/RasPi通信)
 * 1 (D1) : Serial TX (PC/RasPi通信)
 * 2 (D2) : 左エンコーダ A相
 * 3 (D3) : 右エンコーダ A相
 * 4 (D4) : 左エンコーダ B相
 * 5 (D7) : 右エンコーダ B相
 * 6 (D6) : 超音波センサ Echo (HC-SR04)
 * 7 (D5) : 超音波センサ Trig (HC-SR04)
 * 8 (D8) : 右モータ DIR (回転方向)
 * 9 (D9) : 右モータ PWM (速度指令)
 * 10 (D10): [未使用] (PWM出力可能)
 * 11 (D11): 左モータ PWM (速度指令)
 * 12 (D12): 左モータ DIR (回転方向)
 * 13 (D13): 内蔵LED (Status)
 * A0 (14) : バッテリー電圧監視
 * A1 (15) : 操作ボタン 1 (風船選択1)
 * A2 (16) : 操作ボタン 2 (風船選択2)
 * A3 (17) : 操作ボタン 3 (風船選択3)
 * A4 (18) : I2C SDA (Slave通信)
 * A5 (19) : I2C SCL (Slave通信)
 * ======================================================================================
 */

#include "define.h"
#include <Wire.h>

// 状態管理
system_mode_t current_mode = MODE_STANDBY;
int balloon_counts[4] = {0, 0, 0, 0}; // ボタン1,2,3の回数 (index 0は不使用)

// タイムアウト計測用
unsigned long stop_start_time = 0;

// Slaveのボタン状態
bool slave_btn_a = false;
bool slave_btn_b = false;
bool slave_busy  = false;

void setup() {
  // 各モジュール初期化
  io_open();
  encoder_open();
  motor_open();
  raspi_open();
  master_i2c_open();
  sonar_open();
  
  // アクチュエータ等の初期化コマンド送信（念のため）
  delay(1000);

  // --- デバッグモード判定 ---
  Serial.println("Wait 3sec... Press 't':Test, 'b':Balloon Demo");
  unsigned long start = millis();
  while (millis() - start < 3000) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      
      if (c == 't') {
        current_mode = MODE_TEST;
        Serial.println("Entered TEST MODE");
        break;
      } else if (c == 'b') {
        current_mode = MODE_DEMO_BALLOON;
        Serial.println("Entered BALLOON DEMO MODE");
        break;
      } else if (c == 's') { // <--- 追加
        current_mode = MODE_TEST_SERVO;
        Serial.println("Entered SERVO CHECK MODE");
        break;
      }
    }
  }
}

void loop() {
  // Slaveのボタン状態などを更新
  update_slave_status();

  switch (current_mode) {
    case MODE_STANDBY:
      loop_standby();
      break;
    case MODE_PATROL:
      loop_patrol();
      break;
    case MODE_STOP_WAIT:
      loop_stop_wait();
      break;
    case MODE_BALLOON:
      loop_balloon();
      break;
    case MODE_TEST:
      loop_test();
    case MODE_DEMO_BALLOON:
      loop_demo_balloon();
    case MODE_TEST_SERVO:
      loop_test_servo();
      break;
  }
  
  // 制御周期調整
  delay(T_CTRL); 
}

// --- 各モードの処理 ---

// 1. スタンバイモード
void loop_standby() {
  vel_ctrl_set(0, 0); // 停止維持
  vel_ctrl_execute();

  // ボタンA (Slave) で巡回開始
  if (slave_btn_a) {
    Serial.println("Go to PATROL");
    current_mode = MODE_PATROL;
    // パトロール初期化処理が必要ならここに書く
  }
}

// 2. 巡回モード
void loop_patrol() {
  static int phase = 0; // 0:直進, 1:回転
  static bool new_phase = true;
  run_state_t state;
  double spd, dst;

  // 障害物検知 (30cm以内)
  if (sonar_get_dist() < 30.0) {
    Serial.println("Obstacle Detected! Stopping...");
    vel_ctrl_set(0, 0); // 強制停止
    stop_start_time = millis(); // 時間計測開始
    current_mode = MODE_STOP_WAIT;
    return;
  }

  // パトロール動作
  if (new_phase) {
    if (phase == 0) run_ctrl_set(STR, 30.0, 800.0); // 8m直進
    else            run_ctrl_set(ROT, 45.0, 180.0); // 180度回転
    new_phase = false;
  }

  run_ctrl_execute();
  vel_ctrl_execute();
  run_ctrl_get(&state, &spd, &dst);

  // 動作完了したら次のフェーズへ
  if (state == STP) {
    phase = !phase; // 0 <-> 1 切り替え
    new_phase = true;
    delay(500); // 少し休憩
  }
}

// 3. 一時停止＆分岐待ちモード
void loop_stop_wait() {
  vel_ctrl_set(0, 0); // 停止
  vel_ctrl_execute();

  unsigned long elapsed = millis() - stop_start_time;

  // A. 30秒以内：ボタンB待ち
  if (elapsed < 30000) {
    if (slave_btn_b) {
      Serial.println("Button B Pressed -> BALLOON MODE");
      current_mode = MODE_BALLOON;
    }
  } 
  // B. 30秒経過後
  else {
    // 物体がなくなったら巡回再開
    if (sonar_get_dist() > 35.0) { // ヒステリシスを持たせて35cm
      Serial.println("Obstacle Cleared -> Resume PATROL");
      current_mode = MODE_PATROL;
      // ※run_ctrlの状態は維持されているので、途中から再開される
    } else {
      // 物体がある間はボタンB待ちを継続
      if (slave_btn_b) {
        current_mode = MODE_BALLOON;
      }
    }
  }
}

// 4. 風船モード
void loop_balloon() {
  vel_ctrl_set(0, 0);
  vel_ctrl_execute();

  // Slaveが動作中なら何もしないで待つ
  if (slave_busy) return;

  // Masterのボタン1,2,3をチェック (io_get_swXはio.ino定義)
  int target_btn = 0;
  if (io_get_sw1()) target_btn = 1;
  if (io_get_sw2()) target_btn = 2;
  if (io_get_sw3()) target_btn = 3;

  if (target_btn > 0) {
    // 5回制限チェック
    if (balloon_counts[target_btn] >= 5) {
      Serial.println("Limit Reached -> Reset to STANDBY");
      current_mode = MODE_STANDBY;
      // 全カウントリセットするか、再起動まで使えないかは仕様による
      // ここでは「このモード終了」としてスタンバイへ
      return; 
    }

    // カウントアップ
    balloon_counts[target_btn]++;
    Serial.print("Execute Balloon Sequence: "); Serial.println(target_btn);

    // Slaveへコマンド送信 (1, 2, 3)
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(target_btn); 
    Wire.endTransmission();
    
    // コマンドが確実に渡るまで少し待つ
    delay(100); 
    
    // 動作完了まで待つループへ（次のloopでslave_busyチェックで待機）
    return;
  }

  // 何も押されていない時：終了条件チェック
  // Slaveが暇で、かつ物体がない場合 -> 巡回に戻る
  if (!slave_busy && sonar_get_dist() > 35.0) {
     Serial.println("Balloon Task Done & Clear -> PATROL");
     current_mode = MODE_PATROL;
  }
}

// 5. テストモード
void loop_test() {
  // シリアルからコマンドを受け取って test_motor_step_response を呼ぶなど
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'r') { // 'r'un step response
      test_motor_step_response(150, 150);
    }
  }
}

// --- I2C ヘルパー関数 ---
void update_slave_status() {
  // Slave (Address 8) から 1バイト読み込む
  Wire.requestFrom(SLAVE_ADDR, 1);
  
  if (Wire.available()) {
    byte status = Wire.read();
    // bit0: Btn A, bit1: Btn B, bit7: Busy
    slave_btn_a = (status & (1 << 0));
    slave_btn_b = (status & (1 << 1));
    slave_busy  = (status & (1 << 7));
  }
}

// --- 風船デモモード (障害物無視・回数制限なし) ---
void loop_demo_balloon() {
  vel_ctrl_set(0, 0); // その場で停止
  vel_ctrl_execute();

  // Slaveが動作中なら終わるのを待つ
  if (slave_busy) {
    return;
  }

  // ボタン判定
  int target_btn = 0;
  if (io_get_sw1()) target_btn = 1;
  else if (io_get_sw2()) target_btn = 2;
  else if (io_get_sw3()) target_btn = 3;

  // ボタンが押されたら即実行
  if (target_btn > 0) {
    Serial.print("Demo Action: Button "); 
    Serial.println(target_btn);

    // Slaveへコマンド送信
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(target_btn); 
    Wire.endTransmission();
    
    // 確実にbusyフラグが立つまで一瞬待つ & 連打防止
    delay(500); 
  }
}

// --- サーボ単体テストモード ---
void loop_test_servo() {
  vel_ctrl_set(0, 0); // 停止
  
  if (slave_busy) return; // Slaveが動作中なら待つ

  int target_btn = 0;
  if (io_get_sw1()) target_btn = 1;
  if (io_get_sw2()) target_btn = 2;
  if (io_get_sw3()) target_btn = 3;

  if (target_btn > 0) {
    Serial.print("Servo Test: Button "); 
    Serial.println(target_btn);
    
    // コマンド送信 (11, 12, 13 を送ることで区別する)
    send_slave_command(10 + target_btn); 
    
    delay(500); // チャタリング防止
  }
}
