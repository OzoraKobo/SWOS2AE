#include "M5Atom.h"
#include "Attitude.h"

Attitude   attitude(Attitude::LOG_INFO);    // 姿勢情報取得クラスインスタンス生成

float   pitch;                      // 姿勢 ピッチ
float   roll;                       // 姿勢 ロール
float   yaw;                        // 姿勢 ヨー（未使用）
float   arc;                        // 極座標角
float   val;                        // 大きさ
float   temperature;                // 温度
int     disp_period = 1000;         // 温度表示周期[ms]
int     elapseTime = 0;             // 経過時間

// 姿勢情報取得コールバック関数
void attitude_callback(int s)
{
    Attitude::EVENT event = (Attitude::EVENT)s;

    if (event == attitude.EVENT_TEST) {
        Serial.printf("attitude_callback : %d\n", event);
    }
}

void setup()
{
    M5.begin(true, true, true);
    attitude.DispProperties();
    // 姿勢情報取得初期化
    attitude.Init(attitude_callback);
    attitude.DispProperties();
    // 姿勢情報取得開始
    attitude.Start();
    attitude.DispProperties();
}

void loop()
{
    if (elapseTime >= disp_period) {
        // 表示周期時間経過
        // 姿勢情報取得データ取得
        attitude.GetAttitude(&pitch, &roll, &yaw, &arc, &val);

        // 内部温度データ取得
        attitude.GetTemperature(&temperature);
    
        Serial.printf("Attitude : %7.2f, %7.2f, %7.2f, %7.2f, %7.2f,  Temperature : %6.2f\n", pitch, roll, yaw, arc, val, temperature);
        elapseTime = 0;
    }

    delay(25);
    elapseTime += 25;
}
