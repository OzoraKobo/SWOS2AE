/******************************************************************************
 * @file       Attitude.h
 * @brief      姿勢情報取得　ヘッダファイル
 * @version    1.00
 * @author     SONODA Takehiko (OzoraKobo)
 * @details    姿勢情報取得のクラス定義
 * @date       2021/09/09 v1.00 新規作成
 * @par     
 * @copyright  Copyright ©︎ 2021 SONODA Takehiko All rights reserved.
 ******************************************************************************/

#ifndef _ATTITUDE_H_
#define _ATTITUDE_H_

#include <functional>
#include <M5Atom.h>


typedef std::function<void(int)> AttitudeCallback;

class Attitude : public Task
{
public:

    enum RESULT {                           // 姿勢情報結果
        RESULT_SUCCESS = 0,                 // 正常終了
        RESULT_ALREADY_INIT,                // 初期化済
        RESULT_NO_RECV_DATA,                // 受信データなし
        RESULT_ERR_ARGS,                    // 引数エラー
        RESULT_ERR_PARAM,                   // パラメータエラー
        RESULT_ERR_STATE,                   // 状態エラー
        RESULT_ERR_MEM_ALLOC,               // メモリアロケーション失敗
        RESULT_ERR_MISC,                    // その他エラー
        RESULT_NUM                          // 姿勢情報取得値取得結果数
    };

    enum STATUS {                           // 姿勢情報取得状態
        STATUS_CREATED = 0,                 // 姿勢情報取得生成済
        STATUS_INIT,                        // 姿勢情報取得初期化
        STATUS_READY,                       // 姿勢情報取得開始待ち
        STATUS_RUN,                         // 姿勢情報取得動作中
        STATUS_END,                         // 姿勢情報取得終了
        STATUS_FAILED,                      // 姿勢情報取得実行不能
        STATSU_NUM                          // 姿勢情報取得状態数
    };

    enum EVENT {                            // 姿勢情報取得イベント
        EVENT_INIT = 0,                     // 姿勢情報取得初期化
        EVENT_READY,                        // 姿勢情報取得開始待ち
        EVENT_RUN,                          // 姿勢情報取得動作中
        EVENT_END,                          // 姿勢情報取得終了
        EVENT_TEST,                         // 姿勢情報取得テストイベント
        EVENT_NUM                           // 姿勢情報取得イベント数
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
    Attitude(LOG_LEVEL logLevel = LOG_WARNING);
    // デストラクタ
    ~Attitude();

    // [DEBUG] プロパティ表示
    void DispProperties();
    // 姿勢情報取得初期化
    RESULT Init(AttitudeCallback callback = 0, int sample = 200, int period = 5);
    // 姿勢情報取得開始
    RESULT Start();
    // 姿勢情報取得データ取得
    RESULT GetAttitude(float *pfPitch, float *pfRoll, float *pfYaw, float *pfArc, float *pfVal);
    // 内部温度データ取得
    RESULT GetTemperature(float *pfTemp);
    // 姿勢情報取得状態取得
    STATUS GetStatus();

private:
    bool                    init;               // 初期化済フラグ
    int                     _tempSample;        // 平均温度サンプル数
    int                     _acquire_period;    // 姿勢情報取得周期[ms]
    AttitudeCallback        _callback;          // コールバック関数へのポインタ
    float                   *_tempBuff;         // 移動平均温度計算用バッファへのポインタ
    bool                    running;            // タスク駆動中
    double                  pitch;              // 姿勢 ピッチ
    double                  roll;               // 姿勢 ロール
    double                  yaw;                // 姿勢 ヨー（未使用）
    double                  arc;                // 極座標角
    double                  val;                // 大きさ
    double                  r_rand;             // ラジアン → 角度変換係数
    float                   averageTemp;        // 内部温度（移動平均）
    int                     tempBuffIndex;      // 移動平均温度計算用バッファ入出力インデックス
    STATUS                  status;             // 姿勢情報取得状態
    LOG_LEVEL               _logLevel;          // ログ出力レベル

    // 姿勢情報取得タスク関数
    void run(void *data);
    // ログ出力
    void logOutput(LOG_LEVEL logLevel, char *logMsg);
};
#endif /* _ATTITUDE_H_ */
