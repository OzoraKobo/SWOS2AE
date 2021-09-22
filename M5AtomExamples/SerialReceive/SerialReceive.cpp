/******************************************************************************
 * @file       SerialReceive.cpp
 * @brief      シリアル受信
 * @version    1.00
 * @author     SONODA Takehiko (OzoraKobo)
 * @details    標準シリアルポートで受信した文字列をバッファに格納し、通知する
 * @date       2021/09/09 v1.00 新規作成
 * @par     
 * @copyright  Copyright ©︎ 2021 SONODA Takehiko All rights reserved.
 ******************************************************************************/

#include <freertos/FreeRTOS.h>
#include "SerialReceive.h"

#define RECV_MSG_MAX_SIZE           (SERIAL_RECEIVE_BUFF_SIZE - 1)  // 受信メッセージ最大サイズ
#define RECV_BUFF_SIZE              SERIAL_RECEIVE_BUFF_SIZE        // シリアル受信バッファサイズ
#define RECV_MSG_QUEUE_NUM          4                               // シリアル受信メッセージキュー数
#define RECV_MSG_QUEUE_SIZE         RECV_BUFF_SIZE                  // シリアル受信メッセージキューサイズ

SerialReceive::SerialReceive(SerialReceive::LOG_LEVEL logLevel)
{
    // シリアル受信プロパティ初期化
    _logLevel = logLevel;                   // ログ出力レベル
    init = false;                           // 初期化済フラグ
    _echoback = false;                      // エコーバック
    _task_period = 1;                       // タスク駆動周期[ms]
    _callback = 0;                          // コールバック関数へのポインタ
    // シリアル受信バッファメモリ割り当て
    recvBuff = (char *)pvPortMalloc(RECV_BUFF_SIZE);
    if (recvBuff == NULL) {
        // メモリ確保失敗
        status = STATUS_FAILED;
        return;
    }
    memset(recvBuff, 0, RECV_BUFF_SIZE + 1);  // シリアル受信バッファメモリクリア
    // 受信メッセージキュー生成
    queRecvMsg = xQueueCreate(RECV_MSG_QUEUE_NUM, RECV_MSG_QUEUE_SIZE);
    if (queRecvMsg == NULL) {
        // キュー生成失敗
        status = STATUS_FAILED;
        return;
    }

    running = false;                        // タスク駆動中
    status = STATUS_CREATED;                // シリアル受信状態（生成済）

    // ログ出力
    logOutput(LOG_INFO, "SerialReceive object created.\n");
}

SerialReceive::~SerialReceive()
{
    if (queRecvMsg) {
        // シリアル受信バッファメモリ解放
        vQueueDelete(queRecvMsg);
    }

    logOutput(LOG_INFO, "SerialReceive object deleted.\n");
}

// シリアル受信バッファオブジェクト設定値表示
void SerialReceive::DispProperties()
{
    if (_logLevel < LOG_DEBUG) {
        // ログ出力レベルがDEBUG未満
        // デバッグOFF
        return;
    }

    Serial.print("Serial Receiver values of object.\n");

    // 初期化済フラグ
    Serial.printf("init : %d\n", init);
    // エコーバック
    Serial.printf("echoback : %d\n", _echoback);
    // タスク駆動周期[ms]
    Serial.printf("task_period : %d\n", _task_period);
    // コールバック関数へポインタ
    Serial.printf("callback function: %08X\n", _callback);
    // シリアル受信バッファへのポインタ
    Serial.printf("receive buffer : %08X\n", recvBuff);
    // 受信メッセージキューハンドル
    Serial.printf("receive message quiue handle : %08X\n", queRecvMsg);
    // 受信メッセージバイト数
    Serial.printf("received bytes : %d\n", recvBytes);
    // タスク駆動中
    Serial.printf("running : %d\n", running);
    // シリアル受信状態
    Serial.printf("status : %d\n", status);
}

// シリアル受信バッファオブジェクト初期化
SerialReceive::RESULT SerialReceive::Init(bool echoback, SerialReceiveCallback callback)
{
    logOutput(LOG_INFO, "SerialReceive Initialize\n");

    if (init) {
        // 初期化済
        logOutput(LOG_WARNING, "SerialReceive already initialized\n");
        return RESULT_ALREADY_INIT;
    }

    // エコーバック
    _echoback = echoback;

    // コールバック関数へのポインタ
    _callback = callback;

    // シリアル受信バッファプロパティ初期化完了
    init = true;

    // シリアル受信状態
    status = STATUS_INIT;               // シリアル受信状態（初期化）

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

SerialReceive::RESULT SerialReceive::Start()
{
    logOutput(LOG_INFO, "SerialReceive task starting...\n");
    // タスクスタート
    start();

    return RESULT_SUCCESS;
}

// シリアル受信データ取得
SerialReceive::RESULT SerialReceive::GetReceiveData(char *data)
{
    RESULT      result = RESULT_SUCCESS;    // 処理結果
    BaseType_t  queurResult;                // キュー送信の結果

    // 受信メッセージを受信メッセージキューから取得する
    queurResult = xQueueReceive(queRecvMsg, (void *)data, (TickType_t)10);
    if (queurResult != pdPASS) {
        // キュー受信失敗
        // 受信メッセージなし
        return RESULT_NO_RECV_DATA;
    }

    // 受信メッセージ取得成功
    return result;
}

// シリアル受信状態取得
SerialReceive::STATUS SerialReceive::GetStatus()
{
    // シリアル受信状態を返す
    return status;
}

// シリアル受信最大サイズ取得
int SerialReceive::GetRecvMaxSize()
{
    // シリアル受信バッファサイズを返す
    return RECV_BUFF_SIZE;
}

void SerialReceive::run(void *data)
{
    data = nullptr;

    logOutput(LOG_INFO, "SerialReceive task started.\n");

    // ゴミデータを読み捨てる
    while (Serial.available() > 0) {
        char _c = Serial.read();
    }

    // タスク駆動中セット
    running = true;
    // シリアル受信受信待ち
    status = STATUS_RECV_WAIT;

    while (1)
    {
        while (Serial.available() > 0) {
            char _c = Serial.read();
            if (_echoback) {
                // エコーバック有効
                Serial.printf("%c", _c);
            }
            if ((_c == '\r') || (_c == '\n')) {
                // 終端コードを受信
                // 受信メッセージを受信メッセージキューに移す
                postRecvMsgQueue();
            }
            else {
              // シリアル受信バッファに格納する
              recvBuff[recvBytes++] = _c;
            }
            if (recvBytes >= RECV_MSG_MAX_SIZE) {
                // 受信メッセージバイト数が受信メッセージ最大サイズに達した
                // 受信メッセージを受信メッセージキューに移す
                postRecvMsgQueue();
            }    
        }

        delay(_task_period);
    }
}

// ログ出力
void SerialReceive::logOutput(SerialReceive::LOG_LEVEL logLevel, char *logMsg)
{
    if (logLevel <= _logLevel) {
        // ログ出力レベルが規定値以下
        Serial.print(logMsg);
    }
}

// 受信メッセージキュー送信
void SerialReceive::postRecvMsgQueue()
{
    BaseType_t  queurResult;    // キュー送信の結果

    // 受信メッセージを受信メッセージキューに移す
    queurResult = xQueueSend(queRecvMsg, (void *)recvBuff, (TickType_t)10);
    if (queurResult != pdPASS) {
        // 空きキューなし
        logOutput(LOG_WARNING, "SerialReceive queue is full.\n");
        if (_callback) {
            // コールバック関数登録あり
            // シリアル受信メッセージキューフル
            _callback(EVENT_QUEUE_FULL);
        }
    }
    else {
        // キュー送信成功
        if (_callback) {
            // コールバック関数登録あり
            // シリアル受信受信完了
            _callback(EVENT_RECV_COMP);
        }
    }
    // シリアル受信バッファクリア
    memset(recvBuff, 0, RECV_BUFF_SIZE);
    // 受信メッセージバイト数クリア
    recvBytes = 0;
}
