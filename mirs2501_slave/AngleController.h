#ifndef ANGLE_CONTROLLER_H
#define ANGLE_CONTROLLER_H

#include <Arduino.h>

// ==========================================
// 角度位置制御クラス (Arduino UNO版)
// ==========================================
class AngleController {
private:
    // --- 設定値 ---
    double angle_per_pulse; // 1パルスあたりの角度[rad]
    double kp, ki, kd;      // PIDゲイン
    int delay_ms;           // ループ周期[ms]
    
    // --- 制御用変数 ---
    double error_integral;
    double error_before;

    // --- ハードウェア操作用 関数ポインタ ---
    // std::functionの代わりに生の関数ポインタを使用（UNO向け軽量化）
    void (*set_motor_pwm)(int);    // モータPWM出力関数へのポインタ
    long (*get_encoder_count)();   // エンコーダ取得関数へのポインタ
    void (*reset_encoder_count)(); // エンコーダリセット関数へのポインタ

    // 定数
    const double STOP_THRESHOLD = 3.141592653589793 / 180.0; // 停止判定閾値（約1度）

public:
    // コンストラクタ
    AngleController(double ppr, double gear_ratio, double enc_mag, int loop_delay_ms = 10) 
        : delay_ms(loop_delay_ms), error_integral(0.0), error_before(0.0),
          set_motor_pwm(nullptr), get_encoder_count(nullptr), reset_encoder_count(nullptr) {
        
        // パルス→角度変換係数の計算
        angle_per_pulse = 2.0 * 3.141592653589793 / (ppr * enc_mag * gear_ratio);
        
        // デフォルトゲイン設定
        kp = 5.0;
        ki = 0.05;
        kd = 0.0;
    }

    // ハードウェア関数の登録（セットアップ時に必ず呼ぶ）
    void attachHardware(void (*motor_func)(int),
                        long (*encoder_read_func)(),
                        void (*encoder_reset_func)()) {
        set_motor_pwm = motor_func;
        get_encoder_count = encoder_read_func;
        reset_encoder_count = encoder_reset_func;
    }

    // PIDゲインの変更
    void setGains(double p, double i, double d) {
        kp = p;
        ki = i;
        kd = d;
    }

    // 指定角度まで移動して停止（ブロッキング動作）
    void moveToAngle(double target_angle) {
        // 関数が登録されていない場合は何もしない（安全対策）
        if (set_motor_pwm == nullptr || get_encoder_count == nullptr || reset_encoder_count == nullptr) return;

        reset_encoder_count();
        delay(10); 

        error_integral = 0.0;
        error_before = 0.0;

        while (true) {
            // エンコーダ読み取り & 角度計算
            long current_count = get_encoder_count();
            double current_angle = current_count * angle_per_pulse;

            // 誤差計算
            double error_angle = target_angle - current_angle;

            // 停止判定（誤差が閾値以下ならループを抜ける）
            if (abs(error_angle) < STOP_THRESHOLD) {
                set_motor_pwm(0);
                break;
            }

            // I制御（積分）
            error_integral += error_angle;
            
            // PID計算
            double output = (kp * error_angle) + 
                            (ki * error_integral) + 
                            (kd * (error_angle - error_before));

            // PWM制限 (-255 〜 255)
            int output_pwm = (int)output;
            if (output_pwm > 255) output_pwm = 255;
            if (output_pwm < -255) output_pwm = -255;

            // 出力
            set_motor_pwm(output_pwm);

            // 次回用に保存
            error_before = error_angle;

            delay(delay_ms);
        }
    }
};

#endif
