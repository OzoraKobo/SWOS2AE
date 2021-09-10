#include <freertos/FreeRTOS.h>
#include "LED_DisPlayMsg.h"
#include "font.h"

#define TASK_DELAY  100

LED_DisPlayMsg::LED_DisPlayMsg(LED_DisPlayMsg::LOG_LEVEL logLevel)
{
    // LEDメッセージ表示初期化
    _logLevel = logLevel;                   // ログ出力レベル
    init = false;                           // 初期化済フラグ
    _length = 0;                            // 最大表示文字数
    _callback  = 0;                         // コールバック関数へポインタ
    _type = TYPE_IDLE;                      // LEDメッセージ表示タイプ
    _period = 0;                            // １文字の表示時間[ms]
    msgBuff = (char *)0;                    // 表示メッセージ文字列バッファへのポインタ
    size = 0;                               // 表示メッセージ文字列長
    index = 0;                              // メッセージ表示インデックス
    color.r = 0;                            // LED表示メッセージ表示カラー(R)
    color.g = 0;                            // LED表示メッセージ表示カラー(R)
    color.b = 0;                            // LED表示メッセージ表示カラー(R)
    memset(matrix, 0, sizeof (matrix));     // 文字表示マトリクスデータ
    colIndex = 0;                           // 文字表示マトリクスカラムインデックス
    status = STATUS_INIT;                   // LEDメッセージ表示状態（初期化）
    running = false;                        // タスク駆動中
    _task_period = TASK_DELAY;              // タスク駆動周期[ms]

    // ログ出力
    logOutput(LOG_INFO, "LED DisPlayMsg object created.\n");
}

LED_DisPlayMsg::~LED_DisPlayMsg()
{
    if (msgBuff) {
        // 表示メッセージ文字列バッファメモリ解放
        //pvPortFree(msgBuff);
    }
}

// LEDメッセージ表示オブジェクト設定値表示
void LED_DisPlayMsg::DispProperties()
{
    if (_logLevel < LOG_DEBUG) {
        // ログ出力レベルがDEBUG未満
        // デバッグOFF
        return;
    }

    Serial.println("LED DisPlayMsg value of object.");

    // 初期化済フラグ
    Serial.printf("init : %d\n", init);
    // 最大表示文字数
    Serial.printf("params.length : %d\n", _length);
    // コールバック関数へポインタ
    Serial.printf("params.callback : %08X\n", _callback);
    // LEDメッセージ表示タイプ
    Serial.printf("params.type : %d\n", _type);
    // １文字の表示時間[ms]
    Serial.printf("params.period : %d\n", _period);
    // 表示メッセージ文字列
    Serial.printf("msgBuff : %08X\n", msgBuff);
    if (msgBuff) {
        Serial.printf("msg : '%s'\n", msgBuff);
    }
    else {
        Serial.println("msg : None\n");
    }
    // 表示メッセージ文字列長
    Serial.printf("size : %d\n", size);
    // メッセージ表示インデックス
    Serial.printf("index : %d\n", index);
    // LED表示メッセージ表示カラー
    Serial.printf("color : %d, %d, %d\n", color.r, color.g, color.b);
    // タスク駆動周期[ms]
    Serial.printf("_task_period : %d\n", _task_period);
    // タスク駆動中
    Serial.printf("running : %d\n", running);
    // LEDメッセージ表示状態
    Serial.printf("status : %d\n", status);
}

// LEDメッセージ表示オブジェクト初期化
LED_DisPlayMsg::RESULT LED_DisPlayMsg::Init(int length, LED_DisplayMsgCallback callback)
{
    // ログ出力
    logOutput(LOG_INFO, "LED DsipPlayMsg Initialize.\n");

    if (init) {
        // 初期化済
        logOutput(LOG_WARNING, "LED DsipPlayMsg already initialized.\n");
        return RESULT_ALREADY_INIT;
    }

    // 表示メッセージ文字列バッファメモリ確保
    if (length > 0) {
        // 最大表示文字数 > 0
        // 表示メッセージ文字列バッファメモリ確保
        msgBuff = (char *)pvPortMalloc(length + 1);
        memset(msgBuff, 0, length);
        // 最大表示文字数
        _length = length;
    }

    // コールバック関数へのポインタ
    _callback = callback;

    // LED 横×縦サイズ設定
    M5.dis.setWidthHeight(LED_MATRIX_COL, LED_MATRIX_ROW);
 
    // LEDメッセージ表示プロパティ初期化完了
    init = true;

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

// LEDメッセージ表示表示設定
LED_DisPlayMsg::RESULT LED_DisPlayMsg::SetMsg(char *msg, MSG_TYPE type, unsigned char red, unsigned char green, unsigned char blue, int period)
{
    if (msg == 0) {
        // 表示メッセージ未定義
        // 引数エラー
        return RESULT_ERR_ARGS;
    }

    if (type >= TYPE_NUM) {
        // LEDメッセージ表示タイプ 異常
        // パラメータエラー
        return RESULT_ERR_PARAM;
    }

    if ((red > 255) || (green > 255) || (blue > 255)) {
        // カラー指定異常
        // パラメータエラー
        return RESULT_ERR_PARAM;
    }

    // 表示メッセージ文字列長チェック
    uint16_t len = strlen(msg);
    if (len > _length) {
        // 最大文字数超過
        len = _length;
    }
    else if (len == 0) {
        // 文字列なし
        // パラメータエラー
        return RESULT_ERR_PARAM;
    }

    // 表示メッセージ文字列・パラメータ更新
    _type = type;                       // LEDメッセージ表示タイプ
    _period = period;                   // １文字の表示時間[ms]
    size = len;                         // 表示メッセージ文字列長
    memcpy(msgBuff, msg, size);         // 表示メッセージ文字列
    msgBuff[size]  = 0;                 // 表示メッセージ文字列終端
    index = 0;                          // メッセージ表示インデックス
    color.r = red;                      // LED表示メッセージ表示カラー(R)
    color.g = green;                    // LED表示メッセージ表示カラー(G)
    color.b = blue;                     // LED表示メッセージ表示カラー(B)
    memset(matrix, 0, sizeof (matrix)); // 文字表示マトリクスデータ
    colIndex = 0;                       // 文字表示マトリクスカラムインデックス
    status = STATUS_READY;              // LEDメッセージ表示状態（表示開始待ち）
                                        // タスク駆動周期[ms]
    if ((_type == TYPE_NORMAL_1SHOT) || (_type == TYPE_NORMAL_CONT)) {
        // １文字ずつ切り替えて表示（１回表示） or １文字ずつ切り替えて表示する（繰り返し表示）
        _task_period = TASK_DELAY;      // タスク駆動周期[ms] デフォルト周期
    }
    else if ((_type == TYPE_SCROLL_1SHOT) || (_type == TYPE_SCROLL_CONT)) {
        // スクロール表示（１回表示） or スクロール表示（繰り返し表示）
        _task_period = _period / (LED_MATRIX_COL + 1);    // タスク駆動周期[ms] 1文字表示／（LEDマトリクス列数＋１）
    }
    else {
        // その他
        _task_period = TASK_DELAY;      // タスク駆動周期[ms] デフォルト周期
    }

    return RESULT_SUCCESS;
}

LED_DisPlayMsg::RESULT LED_DisPlayMsg::DispStart()
{
    logOutput(LOG_INFO, "LED DsipPlayMsg message display task start.\n");
    // タスクスタート
    start();

    return RESULT_SUCCESS;
}

LED_DisPlayMsg::RESULT LED_DisPlayMsg::DispClear()
{
    M5.dis.clear();
    return RESULT_SUCCESS;
}

void LED_DisPlayMsg::run(void *data)
{
    uint16_t    scroll_col = 0;     // スクロール表示カラムインデックス

    data = nullptr;

    logOutput(LOG_INFO, "LED DisPlayMsg Task started.");

    // 初期化
    elapseTime = 0;   // 経過時間
    index = 0;        // メッセージ表示インデックス
    DispClear();      // LED表示クリア
    // タスク駆動中セット
    running = true;

    while (1)
    {
        // １文字ずつ切り替えて表示
        if ((status == STATUS_READY) || (status == STATUS_RUN)) {
            if (elapseTime == 0) {
                // 初回または１文字の表示時間経過
                if ((_type == TYPE_NORMAL_1SHOT) || (_type == TYPE_NORMAL_CONT)) {
                    // １文字ずつ切り替えて表示（１回表示） or １文字ずつ切り替えて表示する（繰り返し表示）
                    char chr = ' ';     // 表示する文字コード
                    if (index < size) {
                        // 表示メッセージ文字列有効範囲内
                        chr = msgBuff[index];
                    }
                    // 1文字表示
                    dispChr(chr, color);
                    status = STATUS_RUN;
                    // 文字表示インデックスインクリメント
                    index++;
                    if (_type == TYPE_NORMAL_1SHOT) {
                        // １文字ずつ切り替えて表示（１回表示）
                        if (index > size) {
                            // 全文字表示出力完了
                            // LEDメッセージ表示終了
                            status = STATUS_END;
                            if (_callback != 0) {
                                // ユーザーコールバック関数登録あり
                                // LEDメッセージ表示終了イベント
                                _callback(EVENT_END);
                            }
                        }
                    }
                    else if (_type == TYPE_NORMAL_CONT) {
                        // １文字ずつ切り替えて表示する（繰り返し表示）
                        if (index > size) {
                            // LEDメッセージ末尾まで表示終了
                            // 文字表示インデックスを先頭に戻す
                            index = 0;
                            if (_callback != 0) {
                                // ユーザーコールバック関数登録あり
                                // LEDメッセージ末尾まで表示終了イベント
                                _callback(EVENT_EOM);
                            }
                        }
                    }
                }
            }        
            if ((_type == TYPE_SCROLL_1SHOT) || (_type == TYPE_SCROLL_CONT)) {
                // スクロール表示（１回表示） or スクロール表示（繰り返し表示）
                char chr = ' ';     // 表示する文字コード
                if (index < size) {
                    // 表示メッセージ文字列有効範囲内
                    chr = msgBuff[index];
                }
                // 文字スクロール表示
                scrollChr(chr, scroll_col, color);
                status = STATUS_RUN;
                // スクロール表示カラムインデックスインクリメント
                scroll_col++;
                if (scroll_col > LED_MATRIX_COL) {
                    // 1文字分表示終了
                    // スクロール表示カラムインデックス初期化
                    scroll_col = 0;
                    // 文字表示インデックスインクリメント
                    index++;
                }
                if (_type == TYPE_SCROLL_1SHOT) {
                    // スクロール表示（１回表示）
                    if (index > size) {
                        // 全文字表示出力完了
                        // LEDメッセージ表示終了
                        status = STATUS_END;
                        if (_callback != 0) {
                            // ユーザーコールバック関数登録あり
                            // LEDメッセージ表示終了イベント
                            _callback(EVENT_END);
                        }
                    }
                }
                else if (_type == TYPE_SCROLL_CONT) {
                    // スクロール表示（繰り返し表示）
                    if (index > size) {
                        // LEDメッセージ末尾まで表示終了
                        // 文字表示インデックスを先頭に戻す
                        index = 0;
                        if (_callback != 0) {
                            // ユーザーコールバック関数登録あり
                            // LEDメッセージ末尾まで表示終了イベント
                            _callback(EVENT_EOM);
                        }
                    }
                }
            }   
        }
        delay(_task_period);
        elapseTime += _task_period;   // 経過時間加算
        if (elapseTime >= _period) {
            // １文字の表示時間[ms]経過
            // 経過時間クリア
            elapseTime = 0;
        }
    }
}

LED_DisPlayMsg::RESULT LED_DisPlayMsg::dispChr(int8_t chr, CRGB _color)
{
    uint8_t buffImageData[(LED_MATRIX_COL * 3) * LED_MATRIX_ROW + 2];   // LED表示イメージデータバッファ
    uint8_t *ptrBuffImageData = (uint8_t *)0;                           // LED表示イメージデータバッファへのポインタ
    uint8_t *ptrFontData = (uint8_t *)0;                                // フォントデータへのポインタ

    // キャラクタコードからフォントデータへのポインタを取得する
    if ((chr < FONT5X5_START_CODE) || (FONT5X5_END_CODE < chr)) {
        // フォントの表示範囲外
        char    buff[64];
        sprintf(buff, "Error! : Out of scope. [%02X]", chr);
        logOutput(LOG_ERROR, buff);
       return RESULT_ERR_PARAM;
    }
    ptrFontData = (uint8_t *)Font5x5[chr - FONT5X5_START_CODE];

    // LED表示イメージデータバッファにフォントデータを移す
    ptrBuffImageData = buffImageData;
    *ptrBuffImageData++ = LED_MATRIX_COL;   // Width
    *ptrBuffImageData++ = LED_MATRIX_ROW;   // Hight

    for (int column = 0; column < LED_MATRIX_COL; column++) {
        for (int row = 0; row < LED_MATRIX_ROW; row++) {
            int bit = (ptrFontData[row] >> (column)) & 1;
            if (bit) {
                // LED ON
                *ptrBuffImageData++ = _color.r;
                *ptrBuffImageData++ = _color.g;
                *ptrBuffImageData++ = _color.b;
            }
            else {
                // LED OFF
                *ptrBuffImageData++ = 0;
                *ptrBuffImageData++ = 0;
                *ptrBuffImageData++ = 0;
            }
        }
    }
    M5.dis.displaybuff(buffImageData, 0, 0);

    return RESULT_SUCCESS;
}

LED_DisPlayMsg::RESULT LED_DisPlayMsg::scrollChr(int8_t chr, uint16_t col, CRGB _color)
{
    uint8_t buffImageData[(LED_MATRIX_COL * 3) * LED_MATRIX_ROW + 2];     // LED表示イメージデータバッファ
    uint8_t *ptrBuffImageData = (uint8_t *)0;                             // LED表示イメージデータバッファへのポインタ
    uint8_t *ptrFontData = (uint8_t *)0;                                  // フォントデータへのポインタ

    // キャラクタコードからフォントデータへのポインタを取得する
    if ((chr < FONT5X5_START_CODE) || (FONT5X5_END_CODE < chr)) {
        // フォントの表示範囲外
        char    buff[64];
        sprintf(buff, "Error! : Out of scope. [%02X]", chr);
        logOutput(LOG_ERROR, buff);
       return RESULT_ERR_PARAM;
    }
    ptrFontData = (uint8_t *)Font5x5[chr - FONT5X5_START_CODE];

    // 文字表示マトリクスデータを１列左にシフトする
    for (int column = 0; column < (LED_MATRIX_COL - 1); column++) {
        // 列
        for (int row = 0; row < LED_MATRIX_ROW; row++) {
            // 行
            // 1列左にコピー
            matrix[row][column] = matrix[row][column + 1];
        }
    }

    // 文字表示マトリクスデータの最右列にカラムインデックスの指すフォントデータをコピーする
    for (int row = 0; row < LED_MATRIX_ROW; row++) {
        // 行
        if (col < LED_MATRIX_COL) {
            // 文字フォント幅以内
            int bit = (ptrFontData[row] >> (LED_MATRIX_COL - 1 - col)) & 1;
            if (bit) {
                // LED ON
                matrix[row][LED_MATRIX_COL - 1].r = _color.r;
                matrix[row][LED_MATRIX_COL - 1].g = _color.g;
                matrix[row][LED_MATRIX_COL - 1].b = _color.b;
            }
            else {
                // LED OFF
                matrix[row][LED_MATRIX_COL - 1] = 0;
            }
        }
        else {
            // 文字間
            // LED OFF
            matrix[row][LED_MATRIX_COL - 1] = 0;           
        }
    }

    // LED表示イメージデータバッファにフォントデータを移す
    ptrBuffImageData = buffImageData;
    *ptrBuffImageData++ = LED_MATRIX_COL;   // Width
    *ptrBuffImageData++ = LED_MATRIX_ROW;   // Hight

    // 表示を90度回転させるために行と列を入れ替える
    for (int column = 0; column < LED_MATRIX_COL; column++) {
        for (int row = 0; row < LED_MATRIX_ROW; row++) {
            *ptrBuffImageData++ = matrix[row][LED_MATRIX_COL - column - 1].r;
            *ptrBuffImageData++ = matrix[row][LED_MATRIX_COL - column - 1].g;
            *ptrBuffImageData++ = matrix[row][LED_MATRIX_COL - column - 1].b;
        }
    }
    M5.dis.displaybuff(buffImageData, 0, 0);

    return RESULT_SUCCESS;
}

// ログ出力
void LED_DisPlayMsg::logOutput(LED_DisPlayMsg::LOG_LEVEL logLevel, char *logMsg)
{
    if (logLevel <= _logLevel) {
        // ログ出力レベルが規定値以下
        Serial.print(logMsg);
    }
}
