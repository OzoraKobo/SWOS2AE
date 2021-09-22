/******************************************************************************
 * @file       Attitude.cpp
 * @brief      姿勢情報取得
 * @version    1.00
 * @author     SONODA Takehiko (OzoraKobo)
 * @details    加速度・ジャイロセンサ MPU6886 から姿勢情報（Pitch, Roll）および内部温度を取得する
 * @date       2021/09/09 v1.00 新規作成
 * @par     
 * @copyright  Copyright ©︎ 2021 SONODA Takehiko All rights reserved.
 ******************************************************************************/

#include <freertos/FreeRTOS.h>
#include "Attitude.h"

Attitude::Attitude(Attitude::LOG_LEVEL logLevel)
{
    // 姿勢情報取得プロパティ初期化
    _logLevel = logLevel;                   // ログ出力レベル
    init = false;                           // 初期化済フラグ
    _acquire_period = 50;                   // 姿勢情報取得周期[ms]
    _tempSample = 1000 / _acquire_period;   // 平均温度サンプル数
    _callback = 0;                          // コールバック関数へのポインタ
    _tempBuff = (float *)0;                 // 温度移動平均計算用バッファへのポインタ

    // ワーク変数初期化
    pitch = 0.0;                            // 姿勢 ピッチ
    roll = 0.0;                             // 姿勢 ロール
    yaw = 0.0;                              // 姿勢 ヨー（未使用）
    arc = 0.0;                              // 極座標角
    val = 0.0;                              // 合成ベクトルの大きさ
    r_rand = 180 / PI;                      // ラジアン → 角度変換係数
    tempBuffIndex = 0;                      // 温度移動平均計算用バッファ入出力インデックス
    averageTemp = 0.0;                      // 内部温度（移動平均）

    running = false;                        // タスク駆動中
    status = STATUS_CREATED;                // 姿勢情報取得状態（生成済）

    // ログ出力
    logOutput(LOG_INFO, "Attitude information acquisition object created.\n");
}

Attitude::~Attitude()
{
    logOutput(LOG_INFO, "Attitude information acquisition object deleted.\n");
}

// 姿勢情報取得オブジェクト設定値表示
void Attitude::DispProperties()
{
    if (_logLevel < LOG_DEBUG) {
        // ログ出力レベルがDEBUG未満
        // デバッグOFF
        return;
    }

    Serial.print("Serial Receiver values of object.\n");

    // 初期化済フラグ
    Serial.printf("init : %d\n", init);
    // 姿勢情報取得周期[ms][ms]
    Serial.printf("acquisition period : %d\n", _acquire_period);
    // 平均温度サンプル数
    Serial.printf("number of samples : %d\n", _tempSample);
    // コールバック関数へポインタ
    Serial.printf("callback function: %08X\n", _callback);
    // 移動平均温度計算用バッファへのポインタ
    Serial.printf("temperature buffer : %08X\n", _tempBuff);
    // 姿勢 ピッチ
    Serial.printf("ptich : %.2f\n", pitch);
    // 姿勢 ロール
    Serial.printf("roll  : %.2f\n", roll);
    // 姿勢 ヨー
    Serial.printf("yaw   : %.2f\n", yaw);
    // 極座標角
    Serial.printf("arc   : %.2f\n", arc);
    // 大きさ
    Serial.printf("val   : %.2f\n", val);
    // 移動平均温度計算用バッファ入出力インデックス
    Serial.printf("temperature buffer index : %d\n", tempBuffIndex);
    // 内部温度（移動平均）
    Serial.printf("average temperature : %5.2f\n", averageTemp);
    // タスク駆動中
    Serial.printf("running : %d\n", running);
    // 姿勢情報取得状態
    Serial.printf("status : %d\n", status);
}

// 姿勢情報取得オブジェクト初期化
Attitude::RESULT Attitude::Init(AttitudeCallback callback, int sample, int period)
{
    logOutput(LOG_INFO, "Attitude Initialize\n");

    if (init) {
        // 初期化済
        logOutput(LOG_WARNING, "Attitude already initialized\n");
        return RESULT_ALREADY_INIT;
    }

    // IMU初期化
    M5.IMU.Init();

    // 表示メッセージ文字列バッファメモリ確保
    if ((sample > 0) && (period > 0)) {
        // 平均温度サンプル数, 温度センサ値取得周期[ms]正常
        // 平均温度サンプル数
        _tempSample = sample;
        // 温度センサ値取得周期[ms]
        _acquire_period = period;
        // 表示メッセージ文字列バッファメモリ確保
        _tempBuff = (float *)pvPortMalloc(_tempSample * sizeof (float));
        memset(_tempBuff, 0, _tempSample * sizeof (float));
    }
    else {
        // 平均温度サンプル数, 温度センサ値取得周期[ms]不正
        return RESULT_ERR_PARAM;
    }

    // コールバック関数へのポインタ
    _callback = callback;

    // 姿勢情報取得プロパティ初期化完了
    init = true;

    // 姿勢情報取得状態
    status = STATUS_INIT;               // 姿勢情報取得初期化

    // コールバック関数テスト
    if (_logLevel >= LOG_DEBUG) {
        // ログ出力レベルがDEBUG以上
        if (_callback) {
            // コールバック関数登録あり
            _callback(EVENT_TEST);
        }
    }

    return RESULT_SUCCESS;
}

Attitude::RESULT Attitude::Start()
{
    logOutput(LOG_INFO, "Attitude task starting...\n");
    // タスクスタート
    start();

    return RESULT_SUCCESS;
}

// 姿勢情報取得データ取得
Attitude::RESULT Attitude::GetAttitude(float *pfPitch, float *pfRoll, float *pfYaw, float *pfArc, float *pfVal)
{
    // 姿勢情報を出力する
    *pfPitch = (float)pitch;        // 姿勢 ピッチ
    *pfRoll = (float)roll;          // 姿勢 ロール
    *pfYaw = (float)yaw;            // 姿勢 ヨー（未使用）
    *pfArc = (float)arc;            // 極座標角
    *pfVal = (float)val;            // 大きさ

    // 姿勢情報取得データ取得成功
    return RESULT_SUCCESS;
}

// 内部温度データ取得
Attitude::RESULT Attitude::GetTemperature(float *pfTemp)
{
    // 内部温度データを出力する
    *pfTemp = averageTemp;

    // 内部温度データ取得成功
    return RESULT_SUCCESS;
}

// 姿勢情報取得状態取得
Attitude::STATUS Attitude::GetStatus()
{
    // 姿勢情報取得状態を返す
    return status;
}

void Attitude::run(void *data)
{
    float   sumTemp = 0.0;  // 積算温度
    float   curTemp = 0.0;  // 取得内部温度

    logOutput(LOG_INFO, "Attitude task started.\n");

    data = nullptr;

    // 初期化
    tempBuffIndex = 0;      // 移動平均温度計算用バッファ入出力インデックス
    averageTemp = 0.0;      // 温度（移動平均）

    // タスク駆動中セット
    running = true;
    // 姿勢情報取得動作中
    status = STATUS_RUN;

    while (1)
    {
        // IMUから姿勢状態を取得する
        M5.IMU.getAttitude(&pitch, &roll);
        arc = atan2(pitch, roll) * r_rand + 180;
        val = sqrt(pitch * pitch + roll * roll);

        // IMUから内部温度を取得する
        M5.IMU.getTempData(&curTemp);

        // 移動平均温度計算
        // 移動平均温度計算用バッファからsample数前の温度を取得
        float oldTemp = _tempBuff[tempBuffIndex];
        // 積算温度からsample数前の温度を引く
        sumTemp -= oldTemp;
        // 現在温度を積算温度に加える
        sumTemp += curTemp;
        // 平均温度を計算する
        averageTemp = sumTemp / _tempSample;
        // 現在温度を移動平均温度計算用バッファに格納する
        _tempBuff[tempBuffIndex] = curTemp;
        // 移動平均温度計算用バッファ入出力インデックスを更新する
        tempBuffIndex++;
        if (tempBuffIndex >= _tempSample) {
            tempBuffIndex = 0;
            // Grove温度センサ値取得実行中
            status = STATUS_RUN;
        }

        // 姿勢情報取得周期[ms]ウェイト
        delay(_acquire_period);
    }
}

// ログ出力
void Attitude::logOutput(Attitude::LOG_LEVEL logLevel, char *logMsg)
{
    if (logLevel <= _logLevel) {
        // ログ出力レベルが規定値以下
        Serial.print(logMsg);
    }
}
