#ifndef MASTER_DEFINE_H
#define MASTER_DEFINE_H

#include <Arduino.h>
#include <stdint.h>

// ==========================================
// ピン配置定義
// ==========================================

// --- モーター / エンコーダ ---
#define PIN_ENC_A_L      2  // 左エンコーダ A相
#define PIN_ENC_B_L      4  // 左エンコーダ B相
#define PIN_PWM_L       11  // 左モーター PWM
#define PIN_DIR_L       12  // 左モーター 回転方向

#define PIN_ENC_A_R      3  // 右エンコーダ A相
#define PIN_ENC_B_R      7  // 右エンコーダ B相
#define PIN_DIR_R        8  // 右モーター 回転方向
#define PIN_PWM_R        9  // 右モーター PWM

// --- センサー / 入力 ---
#define PIN_SW          10  // メインスイッチ
#define PIN_SW_1        15  // 機能スイッチ1 (A1)
#define PIN_SW_2        16  // 機能スイッチ2 (A2)
#define PIN_SW_3        17  // 機能スイッチ3 (A3)
#define PIN_BATT        14  // バッテリー電圧監視 (A0)
#define PIN_US_TRIG      5  // 超音波センサ Trig
#define PIN_US_ECHO      6  // 超音波センサ Echo

// --- インジケータ ---
#define PIN_LED         13  // 内蔵LED

// ==========================================
// 機構・ハードウェア パラメータ
// ==========================================

// --- 寸法・ギア比 ---
#define R_TIRE           4.0  // タイヤ半径 [cm] (要補正: test_run_ctrl STRモード)
#define D_TIRE          32.0  // タイヤ間隔 [cm] (要補正: test_run_ctrl ROTモード)
#define GEAR_RATIO       1.0  // ギア比 (1/14ギア等の場合に変更)
#define L_R_RATIO        1.0  // 左タイヤに対する右タイヤの回転比補正
#define ENC_RANGE (2048 * 2)  // エンコーダ分解能 (A相変化のみ利用のため2倍)

// --- 電気特性 ---
#define V_RATIO          0.5  // バッテリ入力の分圧比

// ==========================================
// 制御・動作 パラメータ
// ==========================================

#define T_CTRL            10  // 制御ループ周期 [ms]

// --- パトロール動作設定 ---
#define PATROL_SPEED    30.0  // 走行速度 [cm/s]
#define PATROL_DIST_FW 800.0  // 直進距離 [cm] (8m)
#define PATROL_DIST_ROT 180.0 // 回転角度 [deg]
#define OBSTACLE_DIST   30.0  // 障害物停止距離 [cm]

// --- ステップ・バルブ位置設定 (スレーブ連携用) ---
#define POS_X_HOME         0  // X軸 原点
#define POS_X_A         1000  // X軸 位置A
#define POS_X_B         2000  // X軸 位置B
#define POS_X_C         3000  // X軸 位置C
#define STEP_Y_ACT       500  // Y軸 動作ステップ量

// ==========================================
// 通信設定 (I2C)
// ==========================================
#define SLAVE_ADDR         8  // スレーブArduinoのI2Cアドレス

// ==========================================
// 型定義 (Struct / Enum)
// ==========================================

// --- 走行制御状態 ---
typedef enum {
    STP = 0, // 停止
    STR,     // 直進 (Straight)
    ROT      // 回転 (Rotation)
} run_state_t;

// --- システム動作モード ---
typedef enum {
    MODE_STANDBY = 0,   // 待機
    MODE_PATROL,        // パトロールモード
    MODE_STOP_WAIT,     // 一時停止待機
    MODE_BALLOON,       // 風船割りモード
    MODE_TEST,          // テストモード
    MODE_DEMO_BALLOON,  // デモ用風船割り
    MODE_TEST_SERVO     // サーボテスト
} system_mode_t;

// --- 通信データ構造体 ---
// RasPi -> Arduino シリアル受信用
typedef struct {
    uint8_t val[7];     // ヘッダ含む生データ
} serial_data_t;

// 中間デコード用
typedef struct {
    uint8_t val[6];     // データ部
} middle_data_t;

// コマンド解析後データ
typedef struct {
    int16_t val[3];     // 数値データ (signed short)
} command_data_t;

#endif // MASTER_DEFINE_H
