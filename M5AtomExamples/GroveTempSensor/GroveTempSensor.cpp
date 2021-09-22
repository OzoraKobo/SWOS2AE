/******************************************************************************
 * @file       GroveTempSensor.cpp
 * @brief      Grove温度センサ・温度取得
 * @version    1.00
 * @author     SONODA Takehiko (OzoraKobo)
 * @details    Grove温度センサのアナログ出力をAD変換、摂氏温度に変換し、移動平均を求める
 * @date       2021/09/09 v1.00 新規作成
 * @par     
 * @copyright  Copyright ©︎ 2021 SONODA Takehiko All rights reserved.
 ******************************************************************************/

#include <freertos/FreeRTOS.h>
#include "GroveTempSensor.h"

#define B   4275                       // B value of the thermistor
#define R0  100000                     // R0 = 100k

GroveTempSensor::GroveTempSensor(int ainPin, GroveTempSensor::LOG_LEVEL logLevel)
{
    // Grove温度センサ値取得初期化
    _logLevel = logLevel;               // ログ出力レベル
    init = false;                       // 初期化済フラグ
    _ainPin = ainPin;                   // アナログ入力ピン
    _sample = 50;                       // 平均温度サンプル数
    _period = 20;                       // 温度センサ値取得周期[ms]
    _task_period = 20;                  // タスク駆動周期[ms]
    _tempBuff = (float *)0;             // 温度移動平均計算用バッファへのポインタ
    running = false;                    // タスク駆動中
    tempBuffIndex = 0;                  // 温度移動平均計算用バッファ入出力インデックス
    averageTemp = 0.0;                  // 温度（移動平均）
    status = STATUS_CREATED;            // Grove温度センサ値取得状態（生成済）

    // ログ出力
    logOutput(LOG_INFO, "Grove Temperature Sensor object created.\n");
}

GroveTempSensor::~GroveTempSensor()
{
    if (_tempBuff) {
        // 温度移動平均計算用バッファメモリ解放
        //pvPortFree(_tempBuff);
    }
}

// Grove温度センサ値取得オブジェクト設定値表示
void GroveTempSensor::DispProperties()
{
    if (_logLevel < LOG_DEBUG) {
        // ログ出力レベルがDEBUG未満
        // デバッグOFF
        return;
    }

    Serial.println("Grove Temperature Sensor values of object.");

    // 初期化済フラグ
    Serial.printf("init : %d\n", init);
    // アナログ入力ピン
    Serial.printf("analog input pin : %d\n", _ainPin);
    // 平均温度サンプル数
    Serial.printf("number of samples : %d\n", _sample);
    // 温度センサ値取得周期[ms]
    Serial.printf("acquiring period : %d\n", _period);
    // タスク駆動周期[ms]
    Serial.printf("_task_period : %d\n", _task_period);
    // コールバック関数へポインタ
    Serial.printf("callback function: %08X\n", _callback);
    // 移動平均温度計算用バッファへ
    Serial.printf("temperature buffer : %08X\n", _tempBuff);
    // 移動平均温度計算用バッファ入出力インデックス
    Serial.printf("temperature buffer index : %d\n", tempBuffIndex);
    // 温度（移動平均）
    Serial.printf("average temperature : %5.2f\n", averageTemp);
    // タスク駆動中
    Serial.printf("running : %d\n", running);
    // Grove温度センサ値取得状態
    Serial.printf("status : %d\n", status);
}

// Grove温度センサ値取得オブジェクト初期化
GroveTempSensor::RESULT GroveTempSensor::Init(int sample, int period, GroveTempSensorCallback callback)
{
    // ログ出力
    logOutput(LOG_INFO, "Grove Temperature Sensor initialize.\n");

    if (init) {
        // 初期化済
        logOutput(LOG_WARNING, "Grove Temperature Sensor already initialized.\n");
        return RESULT_ALREADY_INIT;
    }

    // 表示メッセージ文字列バッファメモリ確保
    if ((sample > 0) && (period > 0)) {
        // 平均温度サンプル数, 温度センサ値取得周期[ms]正常
        // 平均温度サンプル数
        _sample = sample;
        // 温度センサ値取得周期[ms]
        _period = period;
        _task_period = period;
        // 表示メッセージ文字列バッファメモリ確保
        _tempBuff = (float *)pvPortMalloc(sample * sizeof (float));
        memset(_tempBuff, 0, sample * sizeof (float));
    }
    else {
        // 平均温度サンプル数, 温度センサ値取得周期[ms]不正
        return RESULT_ERR_PARAM;
    }

    // コールバック関数へのポインタ
    _callback = callback;

    // Grove温度センサ値取得プロパティ初期化完了
    init = true;

    // Grove温度センサ値取得状態
    status = STATUS_INIT;               // Grove温度センサ値取得状態（初期化）

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


GroveTempSensor::RESULT GroveTempSensor::Start()
{
    // ログ出力
    logOutput(LOG_INFO, "Grove Temperature Sensor task starting...");
    // タスクスタート
    start();
}

// Grove温度センサ平均温度取得
float GroveTempSensor::GetAverageTmep()
{
    // 温度（移動平均）を返す
    return averageTemp;
}

// Grove温度センサ値取得状態取得
GroveTempSensor::STATUS GroveTempSensor::GetStatus()
{
    // Grove温度センサ値取得状態を返す
    return status;
}

void GroveTempSensor::run(void *data)
{
    float   sumTemp = 0.0;  // 積算温度

    data = nullptr;

    // ログ出力
    logOutput(LOG_INFO, "Grove Temperature Sensor task started.");

    // 初期化
    tempBuffIndex = 0;      // 移動平均温度計算用バッファ入出力インデックス
    averageTemp = 0.0;      // 温度（移動平均）
    // タスク駆動中セット
    running = true;
    // Grove温度センサ値取得開始待ち
    status = STATUS_READY;

    while (1)
    {
        // Grove 温度センサAD変換値取得
        int a = analogRead(_ainPin);
        // 温度センサAD変換値 → 温度変換
        float R = 4096.0 / a - 1.0;
        R = R0 * R;
        float curTemp = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15; // convert to temperature via datasheet

        // 移動平均温度計算
        // 移動平均温度計算用バッファからsample数前の温度を取得
        float oldTemp = _tempBuff[tempBuffIndex];
        // 積算温度からsample数前の温度を引く
        sumTemp -= oldTemp;
        // 現在温度を積算温度に加える
        sumTemp += curTemp;
        // 平均温度を計算する
        averageTemp = sumTemp / _sample;
        // 現在温度を移動平均温度計算用バッファに格納する
        _tempBuff[tempBuffIndex] = curTemp;
        // 移動平均温度計算用バッファ入出力インデックスを更新する
        tempBuffIndex++;
        if (tempBuffIndex >= _sample) {
            tempBuffIndex = 0;
            // Grove温度センサ値取得実行中
            status = STATUS_RUN;
        }

        delay(_task_period);
    }
}

// ログ出力
void GroveTempSensor::logOutput(GroveTempSensor::LOG_LEVEL logLevel, char *logMsg)
{
    if (logLevel <= _logLevel) {
        // ログ出力レベルが規定値以下
        Serial.print(logMsg);
    }
}
