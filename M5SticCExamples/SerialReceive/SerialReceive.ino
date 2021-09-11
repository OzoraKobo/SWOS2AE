// for GROVE temperature sensor v1.2
//   https://wiki.seeedstudio.com/Grove-Temperature_Sensor_V1.2/
#include <M5StickC.h>
#include "SerialReceive.h"


SerialReceive   serialReceiver(SerialReceive::LOG_INFO);    // シリアル受信クラスインスタンス生成

char    seralReceiveBuff[SERIAL_RECEIVE_BUFF_SIZE];         // シリアル受信バッファ

// LEDメッセージ表示コールバック関数
void SerialReceiver_callback(int s)
{
    SerialReceive::EVENT event = (SerialReceive::EVENT)s;

        Serial.printf("SerialReceiver_callback : %d\n", event);
}

void setup()
{
    M5.begin(true, false, true);

    serialReceiver.DispProperties();
    // シリアル受信初期化
    serialReceiver.Init(true, SerialReceiver_callback);
    serialReceiver.DispProperties();
    // シリアル受信開始
    serialReceiver.Start();
    serialReceiver.DispProperties();
}

void loop()
{
    // シリアル受信メッセージチェック
    SerialReceive::RESULT   serialRecvResult = serialReceiver.GetReceiveData(seralReceiveBuff);
    if (serialRecvResult == serialReceiver.RESULT_SUCCESS) {
        // 受信メッセージ取得成功
        //Serial.printf("Received : %s\n", seralReceiveBuff);
        if (strcmp(seralReceiveBuff, "start") == 0) {
            // "start"コマンド
            Serial.printf("*** start ***\n");
        }
        else if (strcmp(seralReceiveBuff, "stop") == 0) {
            // "stop"コマンド
            Serial.printf("*** stop ***\n");
        }
        else {
            // 認識できないコマンド
            Serial.printf("Invalid command : \"%s\"\n", seralReceiveBuff);
        }
    }
    delay(25);
}
