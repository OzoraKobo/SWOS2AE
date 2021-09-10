/****************************************************************
 * 
 * This Example is used to test leds
 * This Example only for M5Atom Matrix!
 * 
 * Arduino tools Setting 
 * -board : M5StickC
 * -Upload Speed: 115200 / 750000 / 1500000
 * 
****************************************************************/
#include "M5Atom.h"
#include "LED_DisPlayMsg.h"

LED_DisPlayMsg  ldm(LED_DisPlayMsg::LOG_DEBUG);     // LEDメッセージ表示クラスインスタンス生成

uint8_t         chrCode = 0x20;
int             dispCount = 0;                      // 表示カウンタ
bool            dispDisable = false;                // 表示更新禁止
int             delayTime = 100;                    // loop毎のディレイ時間(ms)

// LEDメッセージ表示コールバック関数
void ldm_callback(int s)
{
    LED_DisPlayMsg::EVENT event = (LED_DisPlayMsg::EVENT)s;

    //Serial.printf("ldm_callback : %d\n", event);
    if ((event == ldm.EVENT_EOM ) || (event == ldm.EVENT_END)) {
        // LEDメッセージ末尾まで表示終了
        // 表示更新禁止解除
        dispDisable = false;
    }
}

void setup()
{
    LED_DisPlayMsg::RESULT result;     // LEDメッセージ表示結果

    M5.begin(true, false, true);
    delay(50);
    // LEDメッセージ表示初期化
    result = ldm.Init(256, ldm_callback);
    // LED表示メッセージ設定
    result = ldm.SetMsg("WELCOME TO STARTUP WEEKEND!", ldm.TYPE_SCROLL_CONT, 255, 255, 255, 1500);
    // [DEBUG] プロパティ表示
    ldm.DispProperties();
    // LEDメッセージ表示タスクスタート
    ldm.DispStart();
    delay(50);
}    

void loop()
{
    if (M5.Btn.wasPressed())
    {
        // ボタン押下検出
        if (dispDisable == false) {
            //Serial.println("Button Pressed");
            // 表示更新禁止
            dispDisable = true;
            // 表示更新可
            dispCount++;
            // 表示カウンタの値をLEDマトリクスに表示する
            char buff[16];
            sprintf(buff, "%d", dispCount);
            // LED表示メッセージ設定
            ldm.SetMsg(buff, ldm.TYPE_SCROLL_1SHOT, 255, 255, 255, 1500);
        }
    }

    delay(delayTime);
    M5.update();
}
