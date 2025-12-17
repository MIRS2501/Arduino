void operation_check() {
  // --- SW 1: 走行 (マスター自身の仕事) ---
  if (io_get_sw1()) {
    demo_button_run(); 
  }

  // --- SW 2: アーム動作A + 電磁石ON (スレーブへ指令) ---
  if (io_get_sw2()) {
    // 例: ステッピング位置Aへ移動 (コマンド20)
    send_slave_command(20);
    delay(500); // 少し待つ（連打防止）
  }

  // --- SW 3: アーム動作B + 電磁石OFF (スレーブへ指令) ---
  if (io_get_sw3()) {
    // 例: サーボ動作1 (コマンド1)
    send_slave_command(1);
    delay(500);
  }
}
