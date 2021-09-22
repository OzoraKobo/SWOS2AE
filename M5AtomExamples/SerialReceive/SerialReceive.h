/******************************************************************************
 * @file       SerialReceive.h
 * @brief      シリアル受信 ヘッダファイル
 * @version    1.00
 * @author     SONODA Takehiko (OzoraKobo)
 * @details    シリアル受信のクラス定義
 * @date       2021/09/09 v1.00 新規作成
 * @par     
 * @copyright  Copyright ©︎ 2021 SONODA Takehiko All rights reserved.
 ******************************************************************************/

#ifndef _SERIAL_RECEIVE_H_
#define _SERIAL_RECEIVE_H_

#include <functional>
#include <M5Atom.h>

#define SERIAL_RECEIVE_BUFF_SIZE            128         // シリアル受信バッファサイズ

typedef std::function<void(int)> SerialReceiveCallback;

class SerialReceive : public Task
{
public:

    enum RESULT {                           // シリアル受信結果
        RESULT_SUCCESS = 0,                 // 正常終了
        RESULT_ALREADY_INIT,                // 初期化済
        RESULT_NO_RECV_DATA,                // 受信データなし
        RESULT_ERR_ARGS,                    // 引数エラー
        RESULT_ERR_PARAM,                   // パラメータエラー
        RESULT_ERR_STATE,                   // 状態エラー
        RESULT_ERR_MEM_ALLOC,               // メモリアロケーション失敗
        RESULT_ERR_MISC,                    // その他エラー
        RESULT_NUM                          // シリアル受信値取得結果数
    };

    enum STATUS {                           // シリアル受信状態
        STATUS_CREATED = 0,                 // シリアル受信生成済
        STATUS_INIT,                        // シリアル受信初期化
        STATUS_READY,                       // シリアル受信開始待ち
        STATUS_RECV_WAIT,                   // シリアル受信受信待ち
        STATUS_RECVING,                     // シリアル受信受信中
        STATSU_RECV_COMP,                   // シリアル受信受信完了
        STATUS_END,                         // シリアル受信終了
        STATUS_FAILED,                      // シリアル受信実行不能
        STATSU_NUM                          // シリアル受信状態数
    };

    enum EVENT {                            // シリアル受信イベント
        EVENT_INIT = 0,                     // シリアル受信初期化
        EVENT_READY,                        // シリアル受信開始待ち
        EVENT_RUN,                          // シリアル受信中
        EVENT_RECV_COMP,                    // シリアル受信受信完了
        EVENT_END,                          // シリアル受信終了
        EVENT_QUEUE_FULL,                   // シリアル受信メッセージキューフル
        EVENT_TEST,                         // シリアル受信テストイベント
        EVENT_NUM                           // シリアル受信イベント数
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
    SerialReceive(LOG_LEVEL logLevel = LOG_WARNING);
    // デストラクタ
    ~SerialReceive();

    // [DEBUG] プロパティ表示
    void DispProperties();
    // シリアル受信初期化
    RESULT Init(bool echoback = false, SerialReceiveCallback callback = 0);
    // シリアル受信開始
    RESULT Start();
    // シリアル受信データ取得
    RESULT GetReceiveData(char *data);
    // シリアル受信状態取得
    STATUS GetStatus();
    // シリアル受信最大サイズ取得
    int GetRecvMaxSize();

private:
    bool                        init;           // 初期化済フラグ
    bool                        _echoback;      // エコーバック
    int                         _task_period;   // タスク駆動周期[ms]
    SerialReceiveCallback       _callback;      // コールバック関数へのポインタ
    char                        *recvBuff;      // シリアル受信バッファへのポインタ
    QueueHandle_t               queRecvMsg;     // 受信メッセージキューハンドル
    int                         recvBytes;      // 受信メッセージバイト数
    bool                        running;        // タスク駆動中
    STATUS                      status;         // シリアル受信状態
    LOG_LEVEL                   _logLevel;      // ログ出力レベル

    // シリアル受信タスク関数
    void run(void *data);
    // ログ出力
    void logOutput(LOG_LEVEL logLevel, char *logMsg);
    // 受信メッセージキュー送信
    void postRecvMsgQueue();
};
#endif /* _SERIAL_RECEIVE_H_ */
