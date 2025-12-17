void test_encoder() {
  long enc_l, enc_r;
  char str[100];

  while (1) {
    encoder_get(&enc_l, &enc_r);
    sprintf(str, "enc_l = %6ld, enc_r = %6ld\n", enc_l, enc_r);
    Serial.print(str);
    delay(T_CTRL);
  }
}

void test_distance() {
  double dist_l, dist_r;
  char str[100], str_l[10], str_r[10];

  while (1) {
    distance_get(&dist_l, &dist_r);
    sprintf(str, "dist_l = %s, dist_r = %s\n",
            dtostrf(dist_l, 6, 1, str_l),
            dtostrf(dist_r, 6, 1, str_r));
    Serial.print(str);
    delay(T_CTRL);
  }
}

void test_motor(int pwm_l, int pwm_r) {
  double dist_prev_l, dist_prev_r;
  double dist_curr_l, dist_curr_r;
  double vel_l, vel_r;
  unsigned long start_time;
  
  // 初期化：現在の距離を取得しておく
  distance_get(&dist_prev_l, &dist_prev_r);
  
  // モーター回転開始（これがステップ入力になります）
  motor_set(pwm_l, pwm_r);
  start_time = millis();

  // データのヘッダー出力（Excel等に貼り付けやすくするためCSV形式にします）
  Serial.println("Time[ms],PWM_L,Vel_L[cm/s],PWM_R,Vel_R[cm/s]");

  while (1) {
    unsigned long current_time = millis();
    
    // 1. 現在の距離を取得
    distance_get(&dist_curr_l, &dist_curr_r);

    // 2. 速度計算: (距離の変化 / 時間)
    // T_CTRL は ms単位なので 1000.0 を掛けて秒単位に変換
    vel_l = (dist_curr_l - dist_prev_l) / T_CTRL * 1000.0;
    vel_r = (dist_curr_r - dist_prev_r) / T_CTRL * 1000.0;

    // 3. 次回のために現在の距離を保存
    dist_prev_l = dist_curr_l;
    dist_prev_r = dist_curr_r;

    // 4. シリアル出力 (CSV形式: 時間, PWM, 左速度, 右速度)
    Serial.print(current_time - start_time);
    Serial.print(",");
    Serial.print(pwm_l);
    Serial.print(",");
    Serial.print(vel_l); 
    Serial.print(",");
    Serial.print(pwm_r);
    Serial.print(",");
    Serial.println(vel_r);

    // 制御周期待機
    delay(T_CTRL);
  }
}

void test_vel_ctrl(double vel_l, double vel_r) {
  int i = 0;
  char str[100], str_l[10], str_r[10];
  char str_v[10];
  double variance ;
  static long count = 0;
  static int count_max = 1000; //10秒

  vel_ctrl_set(vel_l, vel_r);

  while (1) {
    vel_ctrl_execute();
    if (i >= 10) {
      vel_ctrl_get(&vel_l, &vel_r);
      vel_ctrl_get_vari(&variance);
     sprintf(str, "vel_l = %s, vel_r = %s   variance= %s \n", 
              dtostrf(vel_l, 6, 1, str_l),
              dtostrf(vel_r, 6, 1, str_r),
              dtostrf(variance, 6, 1, str_v));
      Serial.print(str);
      i = 0;
    }
    i++;
    delay(T_CTRL);
    count++;
    if( count > count_max ) break;
  }
  test_motor(0, 0);
}

void test_run_ctrl(run_state_t state, double speed, double dist) {
  int i = 0;
  char str[100], str_dist[10], str_speed[10];
  double dist_ref;

  dist_ref = dist;


  run_ctrl_set(state, speed, dist);

  while (1) {
    run_ctrl_execute();
    vel_ctrl_execute();
    if (i >= 10) {
      run_ctrl_get(&state, &speed, &dist);
      sprintf(str, "state = %s, speed = %s, dist = %s\n",
              ((state == STR) ? "STR" : (state == ROT) ? "ROT" : "STP"),
              dtostrf(speed, 6, 1, str_speed),
              dtostrf(dist, 6, 1, str_dist));
      Serial.print(str);
      i = 0;
    }
    i++;

    if( fabs(dist) >= fabs(dist_ref)) state = STP;
    if( state == STP) vel_ctrl_set(0.0, 0.0); //テスト終了後に停めるとき有効にする

    //if( state == STP ) break;
    delay(T_CTRL);
  }
}

void test_batt() {
  double batt;
  char str[100], str_batt[10];

  while (1) {
    batt = io_get_batt();
    sprintf(str, "volt = %s\n", dtostrf(batt, 4, 2, str_batt));
    Serial.print(str);
    delay(T_CTRL);
  }
}

void test_decode() {
  command_data_t command_data = {30000, -255, 0};
  middle_data_t  middle_data;
  serial_data_t  serial_data;

  while (1) {
    middle_data  = raspi_encode2(command_data);
    serial_data  = raspi_encode1(middle_data);
    middle_data  = raspi_decode1(serial_data);
    command_data = raspi_decode2(middle_data);
    Serial.println(command_data.val[0]);
    Serial.println(command_data.val[1]);
    Serial.println(command_data.val[2]);
    delay(1000);
  }
}

/* * MATLAB解析用データ取得関数
 * 10秒待機 -> 1秒走行 -> 停止
 */
void test_motor_step_response(int pwm_l, int pwm_r) {
  double dist_prev_l, dist_prev_r;
  double dist_curr_l, dist_curr_r;
  double vel_l, vel_r;
  unsigned long start_time;
  int i;
  
  // --- フェーズ1: 準備 (10秒待機) ---
  motor_set(0, 0); // 安全のため停止
  Serial.println("--- READY? ---");
  Serial.println("The motor will start in 10 seconds.");
  
  // LEDを点滅させながら10秒カウントダウン
  for (i = 10; i > 0; i--) {
    Serial.print(i);
    Serial.println(" sec...");
    
    // LED点滅 (チカ、チカ)
    io_set_led(HIGH);
    delay(100);
    io_set_led(LOW);
    delay(900); 
  }
  
  Serial.println("GO!");
  io_set_led(HIGH); // 走行中はLED点灯

  // --- フェーズ2: 走行と計測 (1秒間) ---
  
  // 初期化：直前の距離を取得
  distance_get(&dist_prev_l, &dist_prev_r);
  
  // モーター回転開始
  motor_set(pwm_l, pwm_r);
  start_time = millis();

  // CSVヘッダー出力
  Serial.println("Time[ms],PWM_L,Vel_L[cm/s],PWM_R,Vel_R[cm/s]");

  // 1000ms経過するまでループ
  while (millis() - start_time <= 1000) {
    unsigned long current_time = millis() - start_time;
    
    // 1. 現在の距離を取得
    distance_get(&dist_curr_l, &dist_curr_r);

    // 2. 速度計算 [cm/s]
    vel_l = (dist_curr_l - dist_prev_l) / T_CTRL * 1000.0;
    vel_r = (dist_curr_r - dist_prev_r) / T_CTRL * 1000.0;

    // 3. 次回のために距離を更新
    dist_prev_l = dist_curr_l;
    dist_prev_r = dist_curr_r;

    // 4. データ出力
    Serial.print(current_time);
    Serial.print(",");
    Serial.print(pwm_l);
    Serial.print(",");
    Serial.print(vel_l); 
    Serial.print(",");
    Serial.print(pwm_r);
    Serial.print(",");
    Serial.println(vel_r);

    // 制御周期待機
    delay(T_CTRL);
  }

  // --- フェーズ3: 停止 ---
  motor_set(0, 0);
  io_set_led(LOW);
  Serial.println("--- STOP ---");
  
  // そのまま処理を停止（無限ループ）
  while(1) {
    delay(1000);
  }
}

void demo_button_run() {
  // 設定値
  const double TARGET_SPEED = 30.0;  // 速度 30cm/s
  const double TARGET_DIST  = 100.0; // 距離 100cm

  Serial.println("Start Running...");

  // 指令セット
  run_ctrl_set(STR, TARGET_SPEED, TARGET_DIST);

  run_state_t state;
  double spd, dst;
  
  do {
    run_ctrl_execute();
    vel_ctrl_execute();
    
    // 状態確認
    run_ctrl_get(&state, &spd, &dst);
    delay(T_CTRL);
    
  } while (state != STP); // 停止するまでループ

  Serial.println("Finished.");
}
