#ifndef _GROVE_TEMP_SENSOR_H_
#define _GROVE_TEMP_SENSOR_H_

#include <functional>
#include <M5Atom.h>

typedef std::function<void(int)> GroveTempSensorCallback;

class GroveTempSensor : public Task
{
public:

    enum RESULT {                           // Grove温度センサ値取得値結果
        RESULT_SUCCESS = 0,                 // 正常終了
        RESULT_ALREADY_INIT,                // 初期化済
        RESULT_ALREADY_STARTED,             // タスク起動済
        RESULT_ERR_ARGS,                    // 引数エラー
        RESULT_ERR_PARAM,                   // パラメータエラー
        RESULT_ERR_STATE,                   // 状態エラー
        RESULT_ERR_MEM_ALLOC,               // メモリアロケーション失敗
        RESULT_ERR_MISC,                    // その他エラー
        RESULT_NUM                          // Grove温度センサ値取得結果数
    };

    enum STATUS {                           // Grove温度センサ値取得状態
        STATUS_CREATED = 0,                 // Grove温度センサ値取得生成済
        STATUS_INIT,                        // Grove温度センサ値取得初期化
        STATUS_READY,                       // Grove温度センサ値取得開始待ち
        STATUS_RUN,                         // Grove温度センサ値取得動作中
        STATUS_END,                         // Grove温度センサ値取得終了
        STATUS_FAILED,                      // Grove温度センサ値取得実行不能
        STATSU_NUM                          // Grove温度センサ値取得状態数
    };

    enum EVENT {                            // Grove温度センサ値取得イベント
        EVENT_INIT,                         // Grove温度センサ値取得初期化
        EVENT_READY,                        // Grove温度センサ値取得開始待ち
        EVENT_RUN,                          // Grove温度センサ値取得中
        EVENT_END,                          // Grove温度センサ値取得終了
        EVENT_TEST,                         // Grove温度センサテストイベント
        EVENT_NUM                           // Grove温度センサ値取得イベント数
    };

    enum LOG_LEVEL {                        // ログ出力レベル
        LOG_DISABLED = 0,                   // ログ出力レベル 出力なし
        LOG_ERROR,                          // ログ出力レベル エラー以下
        LOG_WARNING,                        // ログ出力レベル 警告以下
        LOG_INFO,                           // ログ出力レベル 一般情報以下
        LOG_DEBUG,                          // ログ出力レベル デバッグ情報以下
        LOG_NUM                             // ログ出力レベル数
    };

    // コンストラクタ
    GroveTempSensor(int ainPin = 33, LOG_LEVEL logLevel = LOG_WARNING);
    // デストラクタ
    ~GroveTempSensor();

    // [DEBUG] プロパティ表示
    void DispProperties();
    // Grove温度センサ値取得初期化
    RESULT Init(int sampele = 50, int period = 20, GroveTempSensorCallback callback = 0);
    // Grove温度センサ値取得開始
    RESULT Start();
    // Grove温度センサ平均温度取得
    float GetAverageTmep();
    // Grove温度センサ値取得状態取得
    STATUS GetStatus();

private:
    bool                        init;           // 初期化済フラグ
    int                         _ainPin;        // アナログ入力ピン
    int                         _sample;        // 平均温度サンプル数
    int                         _period;        // 温度センサ値取得周期[ms]
    int                         _task_period;   // タスク駆動周期[ms]
    GroveTempSensorCallback     _callback;      // コールバック関数へのポインタ
    float                       *_tempBuff;     // 移動平均温度計算用バッファへのポインタ
    bool                        running;        // タスク駆動中
    int                         tempBuffIndex;  // 移動平均温度計算用バッファ入出力インデックス
    float                       averageTemp;    // 温度（移動平均）
    STATUS                      status;         // Grove温度センサ値取得状態
    LOG_LEVEL                   _logLevel;      // ログ出力レベル

    // Grove温度センサ値取得タスク関数
    void run(void *data);
    // ログ出力
    void logOutput(LOG_LEVEL logLevel, char *logMsg);
};
#endif /* _GROVE_TEMP_SENSOR_H_ */
