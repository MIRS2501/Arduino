#ifndef SLAVE_DEFINE_H
#define SLAVE_DEFINE_H

// ステッピングモーター
#define X_STEP_PIN   2
#define X_DIR_PIN    6  // X DIR
#define Y_STEP_PIN   3
#define Y_DIR_PIN    7  // Y DIR

// 電磁石
#define PIN_SOLENOID 4

// アクチュエータ (バルブ用DCモータ)
#define PIN_ACT_PWM  5  // PWMピン
#define PIN_ACT_DIR  8  // DIRピン

// アクチュエータ用エンコーダ
#define PIN_ACT_ENC_A A0 // PCINT8
#define PIN_ACT_ENC_B A1 // PCINT9

// リミットスイッチ
#define X_LIMIT_PIN  9
#define Y_LIMIT_PIN  10

// サーボモーター
#define PIN_SERVO_1  11
#define PIN_SERVO_2  12
#define PIN_SERVO_3  13

// 操作ボタン
#define PIN_BTN_A    A2 // 巡回開始
#define PIN_BTN_B    A3 // 風船モード移行

// ==========================================
// Actuator Parameters (モータ仕様)
// ==========================================
#define ACT_PPR        12.0  // パルス数
#define ACT_GEAR_RATIO 50.0  // ギア比
#define ACT_ENC_MAG    4.0   // 逓倍

// ==========================================
// System Constants
// ==========================================
#define SLAVE_I2C_ADDR 8
#define BAUD_RATE      115200

#define Y_ACT_STEPS 500

#endif
