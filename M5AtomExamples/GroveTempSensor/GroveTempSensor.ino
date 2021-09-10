// for GROVE temperature sensor v1.2
//   https://wiki.seeedstudio.com/Grove-Temperature_Sensor_V1.2/

#include "M5Atom.h"
#include "GroveTempSensor.h"


int     pin_temperature = 33;       // 温度センサ入力アナログ入力ピン
int     avaraging_sample_num = 50;  // 温度平均化サンプル数
int     acquiring_period = 20;      // 温度センサ値取得周囲
int     disp_period = 4000;         // 温度表示周期[ms]
int     elapseTime = 0;             // 経過時間

GroveTempSensor  gts(pin_temperature, GroveTempSensor::LOG_DEBUG);        // Grove温度センサ値取得クラスインスタンス生成

// LEDメッセージ表示コールバック関数
void gts_callback(int s)
{
    GroveTempSensor::EVENT event = (GroveTempSensor::EVENT)s;

    Serial.printf("gts_callback : %d\n", event);
}

void setup()
{
    M5.begin(true, false, true);
    delay(50);

    // Grove温度センサ値取得初期化
    gts.Init(avaraging_sample_num, acquiring_period, gts_callback);
    // Grove温度センサ値取得開始
    gts.Start();
    gts.DispProperties();
    delay(50);
}

void loop()
{
    float temperature = gts.GetAverageTmep();
    if (elapseTime >= disp_period) {
        Serial.printf("Temp : %5.2f", temperature);
        Serial.println("");
        elapseTime = 0;
    }

    delay(25);
    elapseTime += 25;
}
