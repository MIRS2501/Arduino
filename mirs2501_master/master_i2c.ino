#include <Wire.h>

void master_i2c_open() {
  Wire.begin(); // マスターとして参加
}

// コマンド送信関数
// cmd: 1-3(サーボ), 10-11(電磁石), 20-22(ステッピング)
void send_slave_command(int cmd) {
  Wire.beginTransmission(SLAVE_ADDR);
  Wire.write(cmd);
  Wire.endTransmission();
  
  Serial.print("Sent Command: ");
  Serial.println(cmd);
}
