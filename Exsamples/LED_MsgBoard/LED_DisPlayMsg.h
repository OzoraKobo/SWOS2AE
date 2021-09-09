#ifndef _LED_DISPLAYMSG_H_
#define _LED_DISPLAYMSG_H_

#include <functional>
#include <M5Atom.h>
#include "utility/LED_DisPlay.h"

#define LED_MATRIX_ROW      5               // LEDマトリクス 行数
#define LED_MATRIX_COL      5               // LEDマトリクス 列数

typedef std::function<void(int)> LED_DisplayMsgCallback;

class LED_DisPlayMsg : public Task
{
public:

    enum MSG_TYPE {                         // LEDメッセージ表示タイプ
        TYPE_IDLE = 0,                      // アイドル状態（表示なし）
        TYPE_NORMAL_1SHOT,                  // １文字ずつ切り替えて表示（１回表示）
        TYPE_SCROLL_1SHOT,                  // スクロール表示（１回表示）
        TYPE_NORMAL_CONT,                   // １文字ずつ切り替えて表示する（繰り返し表示）
        TYPE_SCROLL_CONT,                   // スクロール（繰り返し表示）
        TYPE_NUM                            // LEDメッセージ表示タイプ数
    };

    enum RESULT {                           // LEDメッセージ表示結果
        RESULT_SUCCESS = 0,                 // 正常終了
        RESULT_ALREADY_INIT,                // 初期化済
        RESULT_ALREADY_STARTED,             // タスク起動済
        RESULT_NO_RECV_DATA,                // 受信データなし
        RESULT_ERR_ARGS,                    // 引数エラー
        RESULT_ERR_PARAM,                   // パラメータエラー
        RESULT_ERR_STATE,                   // 状態エラー
        RESULT_ERR_MEM_ALLOC,               // メモリアロケーション失敗
        RESULT_ERR_MISC,                    // その他エラー
        RESULT_NUM                          // LEDメッセージ表示結果数
    };

    enum STATUS {                           // LEDメッセージ表示状態
        STATUS_CREATED = 0,                 // LEDメッセージ表示生成済
        STATUS_INIT,                        // LEDメッセージ表示初期化
        STATUS_READY,                       // LEDメッセージ表示開始待ち
        STATUS_RUN,                         // LEDメッセージ表示動作中
        STATUS_END,                         // LEDメッセージ表示終了
        STATUS_FAILED,                      // LEDメッセージ表示実行不能
        STATSU_NUM                          // LEDメッセージ表示状態数
    };

    enum EVENT {                            // LEDメッセージ表示イベント
        EVENT_INIT = 0,                     // LEDメッセージ表示初期化
        EVENT_READY,                        // LEDメッセージ表示開始待ち
        EVENT_RUN,                          // LEDメッセージ表示中
        EVENT_EOM,                          // LEDメッセージ末尾まで表示終了
        EVENT_END,                          // LEDメッセージ表示終了
        EVENT_TEST,                         // テストイベント
        EVENT_NUM                           // LEDメッセージ表示イベント数
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
    LED_DisPlayMsg(LOG_LEVEL logLevel = LOG_WARNING);
    // デストラクタ
    ~LED_DisPlayMsg();

    // [DEBUG] プロパティ表示
    void DispProperties();
    // 初期化_tempBuff
    RESULT Init(int length, LED_DisplayMsgCallback callback);
    // 表示メッセージ設定
    RESULT SetMsg(char *msg, LED_DisPlayMsg::MSG_TYPE type = LED_DisPlayMsg::TYPE_NORMAL_1SHOT, unsigned char red = 255, unsigned char green = 255, unsigned char blue = 255, int period = 1000);
    // メッセージ表示開始
    RESULT DispStart();
    // LED表示クリア
    RESULT DispClear();

private:
    bool                    init;           // 初期化済フラグ
    int                     _length;        // 最大表示文字数
    LED_DisplayMsgCallback  _callback;      // コールバック関数へのポインタ
    MSG_TYPE                _type;          // LEDメッセージ表示タイプ
    int                     _period;        // １文字の表示時間[ms]
    char                    *msgBuff;       // 表示メッセージ文字列バッファへのポインタ
    uint16_t                size;           // 表示メッセージ文字列長
    uint16_t                index;          // 文字表示インデックス
    CRGB                    color;          // LED表示メッセージ表示カラー
    STATUS                  status;         // LEDメッセージ表示状態
    bool                    running;        // タスク駆動中
    uint16_t                elapseTime;     // 経過時間カウンタ
    CRGB                    matrix[LED_MATRIX_ROW][LED_MATRIX_COL];     // 文字表示マトリクスデータ
    uint16_t                colIndex;       // 文字表示マトリクスカラムインデックス
    int                     _task_period;   // タスク駆動周期[ms]
    LOG_LEVEL               _logLevel;      // ログ出力レベル

    // 1文字表示
    RESULT dispChr(int8_t chr, CRGB _color);
    // 文字スクロール表示
    RESULT scrollChr(int8_t chr, uint16_t col, CRGB _color);
    // LEDメッセージ表示タスク関数
    void run(void *data);
    // ログ出力
    void logOutput(LOG_LEVEL logLevel, char *logMsg);

};
#endif /* _LED_DISPLAYMSG_H_ */
