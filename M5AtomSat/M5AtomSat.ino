
/******************************************************************************
 * @file       M5AtomSat.ino
 * @brief      「宇宙プログラミング」デモプログラム M5 ATOM Matrix版
 * @version    1.00
 * @author     SONODA Takehiko (OzoraKobo)
 * @details    人工衛星を模擬しシリアル通信による疑似コマンド入力・テレメトリ出力、および簡単なミッションを行う
 * @date       2021/09/09 v0.10 新規作成 シリアル受信機能のみ
 * @date       2021/09/20 v1.00 疑似コマンド・テレメトリ機能、LEDマトリクス表示機能追加
 * @par     
 * @copyright  なし
 ******************************************************************************/
/******************************************************************************
  * 機能
  * (1) 起動から一定時間後コマンド受付・各種出力の抑制解除
  *     ISSから放出される衛星が30分間電波の発射やアンテナ等機構の展開ができないのを模して
  *     TIMER_CMD_RECV_EN(秒)で指定された時間コマンドの受付やテレメトリ、その他の出力を行わない
  * (2) コマンド機能
  *     シリアルポート(115200bps)から受信した文字列をコマンドとして処理する
  *     1) "tlmon" テレメトリ出力許可
  *        シリアルポートへのテレメトリ出力を再開する
  *     2) "tlmoff" テレメッセージ出力禁止
  *        シリアルポートへのテレメトリ出力を停止する
  *     3) "temp" 内部温度をLEDに表示する
  *        加速度・ジャイロセンサ（MPU6886）内部温度をLEDに表示する
  * (3) テレメトリ出力機能
  *     マイコンの状態をTLM_INTERVAL(秒)毎にシリアルポート(115200bps)に出力する
  *     テレメトリデータの収集(getTelemetryData)は起動後から行うが、
  *     テレメトリ出力(setTelemetryMsg)は起動からIMER_CMD_RECV_EN(秒)経過後に開始する
  *     "tlmon", "tlmoff"コマンドでテレメトリ出力をON/OFFできる
  *     テレメトリ出力内容
  *     1) テレメトリ番号　テレメトリ収集開始からの通し番号
  *     2) 起動からの経過時間（時：分：秒）
  *     3) 姿勢情報 Pitch
  *     4) 姿勢情報 Roll
  *     5) 姿勢情報 Yaw　（MPU6886からは取得不可）
  *     6) 加速度・ジャイロセンサ(MPU6886)内部温度
  * (4) LED秒数ドット表示機能
  *     25個のLEDを用い、0〜49秒を表す(setLedSecDotDisp)
  *     ここで言う秒は、起動からの経過時間を50で割った余りである
  *         0秒      : 全消灯
  *         1〜24秒  : 上から１行ずつ左から右へ１個ずつ点灯していく
  *         25秒     : 全点灯
  *         26〜49秒 : 上から１行ずつ左から右へ１個ずつ消灯していく
  *     温度表示中は秒数ドット表示を行わない
  * (5) LEDメッセージ温度表示機能
  *     加速度・ジャイロセンサの内部温度をLEDマトリスクスにスクロール表示する(dispTemp)
  *     温度カラーテーブル(temp_col_tbl)を用い、音頭によって表示する色カラーを変えることができる
 ******************************************************************************/

#include "M5Atom.h"
#include "utility/M5Timer.h"
#include "SerialReceive.h"
#include "Attitude.h"
#include "LED_DisPlayMsg.h"

// タイマー
M5Timer         timer;                          // M5Timer オブジェクト生成
#define         TIMER_1SEC      1000            // タイマー：1秒
bool            timer_1sec_flag = false;        // 1秒タイマーフラグ
uint32_t        run_time = 0;                   // 起動後の経過時間(秒)

// コマンド受信許可タイマー
#define         TIMER_CMD_RECV_EN   30          // コマンド受信許可タイマー[秒]
bool            cmd_recv_enable = false;        // コマンド受信許可フラグ

// シリアル受信
SerialReceive   serialReceiver(SerialReceive::LOG_INFO);        // シリアル受信クラスインスタンス生成
char            seralReceiveBuff[SERIAL_RECEIVE_BUFF_SIZE];     // シリアル受信バッファ

// 姿勢情報取得
Attitude        attitude(Attitude::LOG_INFO);   // 姿勢情報取得クラスインスタンス生成
float           imu_pitch;                      // 姿勢 ピッチ
float           imu_roll;                       // 姿勢 ロール
float           imu_yaw;                        // 姿勢 ヨー（未使用）
float           imu_arc;                        // 極座標角
float           imu_val;                        // 大きさ
float           imu_temp;                       // IMU（加速度・ジャイロセンサ）温度

// LEDメッセージ表示
LED_DisPlayMsg  ldm(LED_DisPlayMsg::LOG_INFO);  // LEDメッセージ表示クラスインスタンス生成
#define         LED_MSG_MAX_LEN     32          // LEDメッセージ表示最大文字数
#define         LED_MSG_DSIP_TIME   1500        // LEDメッセージ１文字表示時間[ms]      
bool            led_msg_busy = false;           // LEDメッセージ表示ビジーフラグ [true=表示中，false=非表示]

// LEDメッセージ温度表示
#define         TEMP_COL_TBL_DIV    24          // 温度カラーテーブル温度分割数
#define         TEMP_COL_TBL_NUM    25          // 温度カラーテーブル数（TEMP_COL_TBL_DIV + 1）
#define         TEMP_COL_TBL_LOWER  0.0         // 温度カラー表示下限温度
#define         TEMP_COL_TBL_UPPER  48.0        // 温度カラー表示上限温度
const unsigned char   temp_col_tbl[TEMP_COL_TBL_NUM][3] = {
    {   0,      0,	    255 },
    {   0,	    42,	    255 },
    {   0,	    85,	    255 },
    {   0,	    128,	255 },
    {   0,	    170,	255 },
    {   0,	    212,	255 },
    {   0,	    255,	255 },
    {   0,	    255,	213 },
    {   0,	    255,	170 },
    {   0,	    255,	128 },
    {   0,	    255,	85  },
    {   0,	    255,	42  },
    {   0,	    255,	0   },
    {   42,	    255,	0   },
    {   85,	    255,	0   },
    {   128,	255,	0   },
    {   170,	255,	0   },
    {   212,	255,	0   },
    {   255,	255,	0   },
    {   255,	212,	0   },
    {   255,	170,	0   },
    {   255,	128,	0   },
    {   255,	85,	    0   },
    {   255,	43,	    0   },
    {   255,	0,	    0   }
};                                              // 温度カラーテーブル

// LED秒数ドット表示
#define         LED_DOT_DISP_INT    1           // LED秒数ドット表示更新周期[s]
#define         LED_DOT_DISP_LA     50          // LED秒数ドット表示をラップアラウンドするカウント数
#define         LED_DOT_DISP_LA_HF  25          // LED秒数ドット表示をラップアラウンドするカウント数の半数
bool            led_dot_disp_flag = false;      // LED秒数ドット表示更新出力フラグ [true=出力，false=出力待ち]
int             led_dot_disp_cnt = 0;           // LED秒数ドット表示カウンタ
int             led_dot_disp_int_cnt = 0;       // LED秒数ドット表示インターバルカウンタ
bool            led_matrix[LED_MATRIX_ROW][LED_MATRIX_COL];     // LED秒数ドット表示マトリクス

// テレメトリ出力
// TLM_INTERVAL秒毎に姿勢情報と温度をテレメトリとして出力する
#define         TLM_INTERVAL        10          // テレメトリ出力周期[s]
#define         TLM_OUTPUT_MSG_SIZE 80          // テレメトリ出力メッセージバッファサイズ
bool            tlm_output_enable = false;      // テレメトリ出力許可フラグ [true=出力可能, false=出力不可]
bool            tlm_output_flag = false;        // テレメトリ出力フラグ [true=出力，false=出力待ち]
int             tlm_interval_counter = 0;       // テレメトリ出力インターバルカウンタ
int             tlm_counter = 0;                // テレメトリ出力カウンタ
char            tlm_output_msg[TLM_OUTPUT_MSG_SIZE];    // テレメトリ出力メッセージバッファ

/******************************************************************************
 * @fn      timer_func_1sec
 * @brief   1秒周期タイマ割り込みハンドラ関数
 * @param   void
 * @return  void 
 * @sa
 * @detail  タイマーで１秒毎に呼ばれるハンドラ関数、テレメトリ出力、LED秒数ドット表示のタイミングを決める
 ******************************************************************************/
void timer_func_1sec(void)
{
    timer_1sec_flag = true;                     // 1秒タイマーフラグセット
    run_time++;                                 // 起動後の経過時間(秒)インクリメント
    // テレメトリ出力インターバル
    tlm_interval_counter++;                     // テレメトリ出力インターバルカウンタインクリメント
    if (tlm_interval_counter >= TLM_INTERVAL) {
        //　テレメトリ出力周期の時間経過
        tlm_output_flag = true;                 // テレメトリ出力フラグセット
        tlm_interval_counter = 0;               // テレメトリ出力インターバルカウンタクリア
    }
    // LED秒数ドット表示
    led_dot_disp_int_cnt++;                     // テレメトリ出力インターバルカウンタインクリメント
    if (led_dot_disp_int_cnt >= LED_DOT_DISP_INT) {
        // LED秒数ドット表示更新周期[s]の時間経過
        led_dot_disp_cnt++;                     // LED秒数ドット表示カウンタインクリメント
        if (led_dot_disp_cnt >= LED_DOT_DISP_LA) {
            // LED秒数ドット表示をラップアラウンドするカウント数に達した
            led_dot_disp_cnt = 0;               // LED秒数ドット表示カウンタクリア
        }
        // LED秒数ドット表示設定
        setLedSecDotDisp(led_dot_disp_cnt);
    }
}

/******************************************************************************
 * @fn      serialReceiver_callback
 * @brief   シリアル受信コールバック関数
 * @param   int s : シリアル受信イベント番号
 * @return  void 
 * @sa
 * @detail  シリアル受信でイベントが発生したときに呼ばれる
 ******************************************************************************/
void serialReceiver_callback(int s)
{
    SerialReceive::EVENT event = (SerialReceive::EVENT)s;
    // 何も行わない
}

/******************************************************************************
 * @fn      attitude_callback
 * @brief   姿勢情報取得コールバック関数
 * @param   int s : 姿勢情報取得イベント番号
 * @return  void 
 * @sa
 * @detail  姿勢情報取得でイベントが発生したときに呼ばれる
 ******************************************************************************/
void attitude_callback(int s)
{
    Attitude::EVENT event = (Attitude::EVENT)s;
    // 何も行わない
}

/******************************************************************************
 * @fn      ldm_callback
 * @brief   LEDメッセージ表示コールバック関数
 * @param   int s : LEDメッセージ表示イベント番号
 * @return  void 
 * @sa
 * @detail  LEDメッセージ表示でイベントが発生したときに呼ばれる
 ******************************************************************************/
void ldm_callback(int s)
{
    LED_DisPlayMsg::EVENT event = (LED_DisPlayMsg::EVENT)s;

    if ((event == ldm.EVENT_EOM ) || (event == ldm.EVENT_END)) {
        // LEDメッセージ末尾まで表示終了
        // LEDメッセージ表示ビジークリア → LED秒数ドット表示が可能
        led_msg_busy = false;
    }
}

/******************************************************************************
 * @fn      getTelemetryData
 * @brief   テレメトリデータ収集
 * @param   void
 * @return  void 
 * @sa
 * @detail  テレメトリ出力する項目を収集し、変数に格納する
 ******************************************************************************/
void getTelemetryData(void)
{
    // 姿勢情報取得
    attitude.GetAttitude(&imu_pitch, &imu_roll, &imu_yaw, &imu_arc, &imu_val);
    // 内部温度データ取得
    attitude.GetTemperature(&imu_temp);
    // テレメトリカウンタインクリメント
    tlm_counter++;          
}

/******************************************************************************
 * @fn      setTelemetryMsg
 * @brief   テレメトリ出力メッセージ生成
 * @param   char *msg : テレメトリ出力メッセージを格納するバッファへのポインタ
 * @return  void 
 * @sa
 * @detail  テレメトリ出力する項目をASCII文字列に変換する
 ******************************************************************************/
void setTelemetryMsg(char *msg)
{
    // テレメトリ出力メッセージ生成
    // 1) テレメトリNo
    // 2) 起動からの経過時間（時：分：秒）
    // 3) 姿勢情報 Pitch
    // 4) 姿勢情報 Roll
    // 5) 姿勢情報 Yaw　（MPU6886からは取得不可）
    // 6) 内部温度
    int hour = run_time / 3600;                     // 時
    int min  = (run_time - (hour * 3600)) / 60;     // 分
    int sec  = run_time % 60;                       // 秒
    sprintf(msg, "TLM, %d, %02d:%02d:%02d, %6.2f, %6.2f, %6.2f, %6.2f", tlm_counter, hour, min, sec, imu_pitch, imu_roll, imu_yaw, imu_temp);
}

/******************************************************************************
 * @fn      setLedSecDotDisp
 * @brief   LED秒数ドット表示設定
 * @param   int sec : 表示する秒数
 * @return  void 
 * @sa
 * @detail  指定された秒数をLEDマトリクスの点灯ドット数で表す。点灯される秒数は、secを50で割った余りとなる
 ******************************************************************************/
void setLedSecDotDisp(int sec)
{
    // LEDマトリクスで50秒のカウント表示を行うためのLEDマトリクスを設定する
    // 0秒     : 全消灯
    // 1-24秒  : 左上から右および下方向に順に点灯していく
    // 25秒    : 全点灯
    // 26-49秒 : 左上から右および下方向に順に消灯していく
    int unit = LED_MATRIX_ROW * LED_MATRIX_COL;
    int remainder = sec % (unit * 2);
    int count = 0;
    for (int row = 0; row < LED_MATRIX_ROW; row++) {
        for (int col = 0; col < LED_MATRIX_COL; col++) {
            if (remainder < unit) {
                // remainder = 0 〜 24
                if (count < remainder) {
                    led_matrix[row][col] = true;
                }
                else {
                    led_matrix[row][col] = false;
                }
            }
            else {
                // remainder = 25 〜 49
                if (count < (remainder - unit)) {
                    led_matrix[row][col] = false;
                }
                else {
                    led_matrix[row][col] = true;
                }
            }
            count++;
        }
    }
}

/******************************************************************************
 * @fn      dispTemp
 * @brief   LEDマトリクスに内部温度をスクロール表示する
 * @param   void
 * @return  void 
 * @sa
 * @detail  IMU（加速度・ジャイロセンサ）の内部温度をLEDマトリクスにスクロール表示する
 ******************************************************************************/
void dispTemp(void)
{
    float   temp = 0.0;     // 内部温度
    char    msg[8];         // LED表示メッセージバッファ
    int     index;          // カラーテーブルインデックス
    float   offset;         // カラーテーブルインデックス計算用

    // 内部温度データ取得
    attitude.GetTemperature(&temp);

    // 温度の表示カラーを計算する
    if (temp < TEMP_COL_TBL_LOWER) {
        // 下限温度未満
        // カラーテーブルの下端
        index = 0;
    }
    else if (temp >= TEMP_COL_TBL_UPPER) {
        // 上限温度以上
        // カラーテーブルの上端
        index = TEMP_COL_TBL_DIV - 1;
    }
    else {
        // 下限〜上限温度範囲内
        // 温度を下限〜上限温度の範囲で正規化する
        index = (int)(((temp - TEMP_COL_TBL_LOWER) / (TEMP_COL_TBL_UPPER - TEMP_COL_TBL_LOWER)) * TEMP_COL_TBL_DIV);
    }

    // 内部温度をLEDに出力する
    sprintf(msg, "%5.1f", temp);
    ldm.SetMsg(msg, ldm.TYPE_SCROLL_1SHOT, temp_col_tbl[index][0], temp_col_tbl[index][1], temp_col_tbl[index][2], 1500);
    // LEDメッセージ表示ビジー
    led_msg_busy = true;
}

/******************************************************************************
 * @fn      setup
 * @brief   起動時処理
 * @param   void
 * @return  void 
 * @sa
 * @detail  Use it to initialize variables, pin modes, start using libraries, etc. 
 *          This function will only run once, after each powerup or reset of the Arduino board.
 ******************************************************************************/
void setup()
{
    // M5 スタート
    //   Serial  : ON
    //   I2C     : ON
    //   Display : ON
    M5.begin(true, true, true);
    delay(50);      // 50msウェイト

    // 1秒周期タイマ割り込みスタート
    timer.setInterval(TIMER_1SEC, timer_func_1sec);

    // 姿勢情報取得初期化
    attitude.Init(attitude_callback);
    // 姿勢情報取得開始
    attitude.Start();

    // LEDメッセージ表示初期化
    ldm.Init(LED_MSG_MAX_LEN, ldm_callback);
    // LED表示メッセージ設定
    ldm.SetMsg(" ", ldm.TYPE_SCROLL_1SHOT, 255, 255, 255, 1500);
    // LEDメッセージ表示ビジー
    led_msg_busy = true;

    // LED秒数ドット表示初期化
    for (int row = 0; row < LED_MATRIX_ROW; row++) {
        for (int col = 0; col < LED_MATRIX_COL; col++) {
            led_matrix[row][col] = false;
        }
    }
}

/******************************************************************************
 * @fn      loop
 * @brief   ループ毎処理
 * @param   void
 * @return  void 
 * @sa
 * @detail  this function does precisely what its name suggests, and loops consecutively, 
 *          allowing your program to change and respond. 
 ******************************************************************************/
void loop()
{
    // M5Timer処理
    timer.run();

    if (tlm_output_flag == true) {
        // テレメトリ出力フラグセット
        // テレメトリデータ収集
        getTelemetryData();
        if (tlm_output_enable == true) {
            // テレメトリ出力許可フラグセット
            // テレメトリ出力メッセージ生成
            setTelemetryMsg(tlm_output_msg);
            // テレメトリ出力メッセージ送信
            Serial.println(tlm_output_msg);
        }
        // テレメトリ出力フラグクリア
        tlm_output_flag = false;
    }

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
                // テレメトリ出力許可フラグセット
                tlm_output_enable = true;
                // LEDメッセージ表示タスクスタート
                ldm.DispStart();
           }
        }
        else {
            // コマンド受信可能
            // LED秒数ドット表示
            if (led_msg_busy == false) {
                // LEDメッセージ非表示
                // LED秒数ドット表示更新
                ldm.SetLedMatrix((bool *)led_matrix, 0, 0, 255);
            }

            // シリアル受信メッセージチェック
            SerialReceive::RESULT   serialRecvResult = serialReceiver.GetReceiveData(seralReceiveBuff);
            if (serialRecvResult == serialReceiver.RESULT_SUCCESS) {
                // 受信メッセージ取得成功
                //Serial.printf("Received : %s\n", seralReceiveBuff);
                if (strcmp(seralReceiveBuff, "tlmon") == 0) {
                    // "tlmon"コマンド　テレメトリ出力許可
                    // テレメトリ出力許可フラグセット
                    tlm_output_enable = true;
                }
                else if (strcmp(seralReceiveBuff, "tlmoff") == 0) {
                    // "tlmoff"コマンド　テレメトリ出力 禁止
                    // テレメトリ出力許可フラグクリア
                    tlm_output_enable = false;
                }
                else if (strcmp(seralReceiveBuff, "temp") == 0) {
                    // "temp"コマンド 内部温度をLEDに表示する
                    dispTemp();
                }
                else {
                    // 認識できないコマンド
                    Serial.printf("Invalid command : \"%s\"\n", seralReceiveBuff);
                }
            }
        }
    }

    if (M5.Btn.wasPressed())
    {
        // ボタン押下検出
        if (cmd_recv_enable == true) {
            // コマンド受信可能
            // "temp"コマンド 内部温度をLEDに表示する
            dispTemp();
        }
        
    }
    M5.update();
}
