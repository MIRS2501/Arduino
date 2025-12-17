// 指定されたモード(直進/回転)で、安全確認しながら移動する関数
void move_safe(run_state_t mode, double speed, double target_val) {
  
  // 1. 移動指令をセット
  run_ctrl_set(mode, speed, target_val);

  run_state_t state;
  double spd_curr, dist_curr;
  double dist_sonar;

  // 2. 目的地に到着するまでループ (STPになるまで)
  do {
    // --- 障害物チェック ---
    dist_sonar = sonar_get_dist();

    // 30cm以内に物体がある場合
    if (dist_sonar < OBSTACLE_DIST) {
      // 一旦モータを強制停止 (run_ctrlの計算は一時無視)
      vel_ctrl_set(0, 0); 
      
      // 物体がなくなるまでここで無限ループして待機
      // ※チャタリング防止のため、再開は少し余裕を見て 35cm とする
      while (sonar_get_dist() < (OBSTACLE_DIST + 5.0)) {
        delay(100); // 0.1秒ごとに確認
      }
      
      // 物体がいなくなったらループを抜ける
      // (run_ctrlは目標距離を覚えているので、次のループから勝手に再開される)
    }

    // --- 通常の制御 ---
    run_ctrl_execute(); // 位置制御の計算
    vel_ctrl_execute(); // 速度制御の実行
    
    // 状態更新
    run_ctrl_get(&state, &spd_curr, &dist_curr);
    
    delay(T_CTRL); // 制御周期待ち

  } while (state != STP); // 目標に到達してSTPになるまで繰り返す
}

// メインのパトロール関数
void patrol_loop() {
  // 1. 直進 8メートル
  move_safe(STR, PATROL_SPEED, PATROL_DIST_FW);
  
  delay(500); // 動作の合間に少し休憩

  // 2. 回転 180度
  move_safe(ROT, 45.0, PATROL_DIST_ROT); // 回転速度は45deg/sくらい推奨
  
  delay(500);
}
