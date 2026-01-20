#ifndef SLAVE_DEFINE_H
#define SLAVE_DEFINE_H

#include <Arduino.h>

// ==========================================
// ピン配置定義
// ==========================================

// --- ステッピングモーター (X軸 / Y軸) ---
#define PIN_STEP_X       2  // X軸 ステップ
#define PIN_DIR_X        6  // X軸 回転方向
#define PIN_STEP_Y       3  // Y軸 ステップ
#define PIN_DIR_Y        7  // Y軸 回転方向

// --- DCアクチュエータ (バルブ開閉) ---
#define PIN_ACT_PWM      5  // モーター PWM
#define PIN_ACT_DIR      8  // モーター 回転方向
#define PIN_ACT_ENC_A   A0  // エンコーダ A相 (PCINT8)
#define PIN_ACT_ENC_B   A1  // エンコーダ B相 (PCINT9)

// --- サーボモーター ---
#define PIN_SERVO_1     11  // ハンド/アーム用 1
#define PIN_SERVO_2     12  // ハンド/アーム用 2
#define PIN_SERVO_3     13  // ハンド/アーム用 3

// --- その他アクチュエータ ---
#define PIN_SOLENOID     4  // 電磁石 (吸着用)

// --- センサー / スイッチ ---
#define PIN_LIMIT_X      9  // X軸 リミットスイッチ
#define PIN_LIMIT_Y     10  // Y軸 リミットスイッチ
#define PIN_BTN_A       A2  // 操作ボタンA (巡回開始)
#define PIN_BTN_B       A3  // 操作ボタンB (風船モード)

// ==========================================
// 機構・ハードウェア パラメータ
// ==========================================

// --- DCアクチュエータ仕様 ---
#define ACT_PPR         12.0  // モーター単体のパルス数
#define ACT_GEAR_RATIO  50.0  // ギア比
#define ACT_ENC_MAG      4.0  // エンコーダ逓倍数 (4逓倍)

// --- ステッピングモーター動作設定 ---
#define Y_ACT_STEPS      2850  // Y軸 動作ステップ量

// ==========================================
// システム・通信設定
// ==========================================
#define SLAVE_I2C_ADDR     8  // I2Cスレーブアドレス
#define BAUD_RATE     115200  // シリアル通信速度

// ==========================================
// 動作設定パラメータ
// ==========================================

#define HOMING_SPEED_X   -500  // X軸の戻る速度 (例: -500)
#define HOMING_SPEED_Y   -500  // Y軸の戻る速度 (例: -500)

// X軸のリミットスイッチ探索タイムアウト [ms]
#define TIMEOUT_X_HOME  15000 

// Y軸のリミットスイッチ探索タイムアウト [ms]
#define TIMEOUT_Y_HOME   5000

#endif // SLAVE_DEFINE_H
