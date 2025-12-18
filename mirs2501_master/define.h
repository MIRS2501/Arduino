/* 変数型の定義 */
typedef enum {
  STP = 0,
  STR,
  ROT
} run_state_t;

typedef struct {
  unsigned char val[7];
} serial_data_t;

typedef struct {
  unsigned char val[6];
} middle_data_t;

typedef struct {
  signed short val[3];
} command_data_t;

/* ピン配置 */
#define PIN_ENC_A_L  2
#define PIN_ENC_B_L  4
#define PIN_ENC_A_R  3
#define PIN_ENC_B_R  7
#define PIN_DIR_R    8
#define PIN_PWM_R    9
#define PIN_DIR_L   12
#define PIN_PWM_L   11
#define PIN_SW      10
#define PIN_LED     13
#define PIN_BATT    14
#define PIN_SW_1    15 
#define PIN_SW_2    16 
#define PIN_SW_3    17
#define PIN_US_TRIG  5
#define PIN_US_ECHO  6

// パトロール設定
#define PATROL_SPEED     30.0  // 走行速度 [cm/s]
#define PATROL_DIST_FW   800.0 // 直進距離 [cm] (8メートル)
#define PATROL_DIST_ROT  180.0 // 回転角度 [deg]
#define OBSTACLE_DIST    30.0  // 停止する距離 [cm]

// スレーブのアドレス定義 (0x08など適当な値)
#define SLAVE_ADDR 8

/* パラメータ */
// 動作周期 [ms]
#define T_CTRL 10 

// タイヤ半径 [cm] 
// test_run_ctrl()のSTRモードでの走行距離もともに補正する
#define R_TIRE     4.0 

// タイヤ間隔 [cm]
// test_run_ctrl()のROTモードでの回転角をもとに補正する
#define D_TIRE    32.0

// エンコーダ分解能 (A相立上り/立下りを利用するため2倍)
#define ENC_RANGE (2048*2) 

// ギア比　（該当するものを選択）
// 1/14 ギア  
#define GEAR_RATIO 1.0

// 左タイヤに対する右タイヤの回転比
#define L_R_RATIO  1.0 

// バッテリ入力の分圧比
#define V_RATIO 0.5 

// ステップ位置の定義
#define POS_X_HOME 0
#define POS_X_A    1000
#define POS_X_B    2000
#define POS_X_C    3000

// Y軸の動作量
#define STEP_Y_ACT 500

// 状態定義
typedef enum {
  MODE_STANDBY = 0,
  MODE_PATROL,
  MODE_STOP_WAIT,
  MODE_BALLOON,
  MODE_TEST
} system_mode_t;

// Slaveアドレス
#define SLAVE_ADDR 8
