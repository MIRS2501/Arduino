#ifndef ANGLE_CONTROLLER_H
#define ANGLE_CONTROLLER_H

#include <Arduino.h>

class AngleController {
private:
    double angle_per_pulse;
    double kp, ki, kd;
    int delay_ms;
    
    // 追加: 制御パラメータ
    int min_pwm;           // モーターが回り始める最小PWM (不感帯補償)
    double integral_limit; // 積分項の上限 (暴走防止)

    double error_integral;
    double error_before;

    // 関数ポインタ
    void (*set_motor_pwm)(int);
    long (*get_encoder_count)();
    void (*reset_encoder_count)();

    const double STOP_THRESHOLD = 0.15; // 停止判定誤差 [rad] (約3度)

public:
    AngleController(double ppr, double gear_ratio, double enc_mag, int loop_delay_ms = 10) 
        : delay_ms(loop_delay_ms), error_integral(0.0), error_before(0.0),
          set_motor_pwm(nullptr), get_encoder_count(nullptr), reset_encoder_count(nullptr) {
        
        angle_per_pulse = 2.0 * 3.1415926535 / (ppr * enc_mag * gear_ratio);
        
        // デフォルト値
        kp = 100.0; ki = 0.0; kd = 0.0;
        min_pwm = 0;
        integral_limit = 1000.0;
    }

    void attachHardware(void (*motor_func)(int), long (*encoder_read_func)(), void (*encoder_reset_func)()) {
        set_motor_pwm = motor_func;
        get_encoder_count = encoder_read_func;
        reset_encoder_count = encoder_reset_func;
    }

    // パラメータ設定用関数 (min_pwm と i_limit を追加)
    void setParams(double p, double i, double d, int minimum_pwm, double i_limit) {
        kp = p;
        ki = i;
        kd = d;
        min_pwm = minimum_pwm;
        integral_limit = i_limit;
    }

    // 移動関数 (大幅改変)
    void moveToAngle(double target_angle) {
        if (!set_motor_pwm || !get_encoder_count) return;

        error_integral = 0.0;
        error_before = 0.0;
        
        unsigned long start_time = millis();
        unsigned long stable_start_time = 0; // 安定し始めた時刻
        bool is_stable = false;

        // タイムアウト設定 (5秒)
        const unsigned long TIMEOUT_MS = 5000;

        while (true) {
            // 1. タイムアウト処理
            if (millis() - start_time > TIMEOUT_MS) {
                set_motor_pwm(0);
                Serial.println(">> PID Timeout!");
                break;
            }

            // 2. 現在値取得
            double current_angle = get_encoder_count() * angle_per_pulse;
            double error = target_angle - current_angle;

            // 3. 安定判定ロジック (目標付近に 200ms 留まったら完了)
            if (abs(error) < STOP_THRESHOLD) {
                if (!is_stable) {
                    is_stable = true;
                    stable_start_time = millis();
                } else if (millis() - stable_start_time > 200) {
                    set_motor_pwm(0); // 完全停止
                    break; // ループを抜ける
                }
            } else {
                is_stable = false;
            }

            // 4. PID計算
            error_integral += error;
            
            // 【重要】積分の暴走ガード (Windup Guard)
            if (error_integral > integral_limit) error_integral = integral_limit;
            if (error_integral < -integral_limit) error_integral = -integral_limit;

            double p_term = kp * error;
            double i_term = ki * error_integral;
            double d_term = kd * (error - error_before);
            
            double output = p_term + i_term + d_term;

            // 5. モーター出力決定
            int pwm = (int)output;

            // 【重要】不感帯補償 (Min PWM)
            // 計算上のPWMが小さくても、min_pwm以下なら底上げして回す
            if (pwm > 0) {
                if (pwm < min_pwm) pwm = min_pwm;
                if (pwm > 255) pwm = 255;
            } else if (pwm < 0) {
                if (pwm > -min_pwm) pwm = -min_pwm;
                if (pwm < -255) pwm = -255;
            }

            // 安定判定に入っている間は、ブレーキをかけるために出力を弱める、または0にする工夫も可能
            // ここではシンプルに出力する
            set_motor_pwm(pwm);

            error_before = error;
            delay(delay_ms);
        }
    }
};

#endif
/*#ifndef ANGLE_CONTROLLER_H
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
    const double STOP_THRESHOLD = 3.14159 / 180.0; // 停止判定閾値（約1度）

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
    pur
    /*void moveToAngle(double target_angle) {
        // 関数が登録されていない場合は何もしない（安全対策）
        if (set_motor_pwm == nullptr || get_encoder_count == nullptr || reset_encoder_count == nullptr) return;

        delay(10); 

        error_integral = 0.0;
        error_before = 0.0;

        unsigned long start_time = millis();

        while (true) {
            if (millis() - start_time > 2000) {
                 set_motor_pwm(0);
                 break;
            }
          
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

#endif*/
