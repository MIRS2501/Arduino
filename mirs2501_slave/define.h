#ifndef SLAVE_DEFINE_H
#define SLAVE_DEFINE_H

#include <Arduino.h>

// ==========================================
// ピン配置定義
// ==========================================
#define PIN_STEP_X       2  
#define PIN_DIR_X        6  
#define PIN_STEP_Y       3  
#define PIN_DIR_Y        7  

#define PIN_ACT_PWM      5  
#define PIN_ACT_DIR      8  
#define PIN_ACT_ENC_A   A0  
#define PIN_ACT_ENC_B   A1  

#define PIN_SERVO_1     11  
#define PIN_SERVO_2     12  
#define PIN_SERVO_3     13  

#define PIN_SOLENOID     4  

#define PIN_LIMIT_X      9  
#define PIN_LIMIT_Y     10  
#define PIN_BTN_A       A2  
#define PIN_BTN_B       A3  

// ==========================================
// 機構・ハードウェア パラメータ
// ==========================================
#define ACT_PPR         12.0  
#define ACT_GEAR_RATIO  50.0  
#define ACT_ENC_MAG      4.0  

#define Y_ACT_STEPS      -2350  

// ==========================================
// システム設定
// ==========================================
#define SLAVE_I2C_ADDR     8  
#define BAUD_RATE     115200  

// ==========================================
// 動作設定パラメータ (ここを調整！)
// ==========================================

// 原点復帰の速度 (負の値にすると逆回転)
// X軸: 逆走する場合は符号を反転させてください (例: 1000 ⇔ -1000)
#define HOMING_SPEED_X   -6000  

// Y軸: 逆走する場合は符号を反転させてください
#define HOMING_SPEED_Y   1000  

// タイムアウト設定 [ms]
#define TIMEOUT_X_HOME  15000 
#define TIMEOUT_Y_HOME   5000

#endif // SLAVE_DEFINE_H
