
#include "M5Atom.h"
#include "utility/M5Timer.h"
#include "SerialReceive.h"

// タイマー
M5Timer     timer;                      // M5Timer オブジェクト生成
#define     TIMER_1SEC      1000        // タイマー：1秒
bool        timer_1sec_flag = false;    // 1秒タイマーフラグ
uint32_t    run_time = 0;               // 起動後の経過時間(秒)

// コマンド受信許可タイマー
#define     TIMER_CMD_RECV_EN   180     // コマンド受信許可タイマー[秒]
bool        cmd_recv_enable = false;    // コマンド受信許可フラグ

// シリアル受信
SerialReceive   serialReceiver(SerialReceive::LOG_INFO);        // シリアル受信クラスインスタンス生成
char            seralReceiveBuff[SERIAL_RECEIVE_BUFF_SIZE];     // シリアル受信バッファ

// 1秒周期タイマ割り込みハンドラ関数
void timer_func_1sec(void)
{
    run_time++;
    //Serial.printf("%d\n", run_time);
    timer_1sec_flag = true;             // 1秒タイマーフラグセット
}

void setup()
{
    // M5 スタート
    //   Serial  : ON
    //   I2C     : ON
    //   Display : ON
    M5.begin(true, true, true);

    // 1秒周期タイマ割り込みスタート
    timer.setInterval(TIMER_1SEC, timer_func_1sec);
}

void loop()
{
    // M5Timer処理
    timer.run();


    if (timer_1sec_flag == true) {
        // 1秒周期処理
        timer_1sec_flag = false;         // 1秒タイマーフラグクリア
        // コマンド受信許可タイマー時間経過チェック
        if (cmd_recv_enable == false) {
            // コマンド受信不可
            if (run_time >= TIMER_CMD_RECV_EN) {
                // コマンド受信許可タイマー時間に達した
                // シリアル受信初期化
                serialReceiver.Init(false);
                // シリアル受信開始
                serialReceiver.Start();
                // コマンド受信許可フラグセット
                cmd_recv_enable = true;
            }
        }
    }

    M5.update();
}
