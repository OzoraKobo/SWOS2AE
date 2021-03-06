# 「宇宙プログラミング」デモプログラム M5 ATOM Matrix版
* 人工衛星を模擬しシリアル通信による疑似コマンド入力・テレメトリ出力、および簡単なミッションを行います

## 機能

### (1) 起動から一定時間後コマンド受付・各種出力の抑制解除
* ISSから放出される衛星が30分間電波の発射やアンテナ等機構の展開ができないのを模してTIMER_CMD_RECV_EN(秒)で指定された時間コマンドの受付やテレメトリ、その他の出力を行わなくする機能です。

### (2) コマンド機能
* シリアルポート(115200bps)から受信した文字列をコマンドとして処理します
* コマンド
  * "tlmon" テレメトリ出力許可
    * シリアルポートへのテレメトリ出力を再開します
  * "tlmoff" テレメッセージ出力禁止
    * シリアルポートへのテレメトリ出力を停止します
  * "temp" 内部温度をLEDに表示する
    * 加速度・ジャイロセンサ（MPU6886）内部温度をLEDに表示します

### (3) テレメトリ出力機能
* マイコンの状態をTLM_INTERVAL(秒)毎にシリアルポート(115200bps)に出力します
* テレメトリデータの収集(getTelemetryData)は起動後から行いますが、テレメトリ出力(setTelemetryMsg)は起動からIMER_CMD_RECV_EN(秒)経過後に開始します
* "tlmon", "tlmoff"コマンドでテレメトリ出力をON/OFFできます
* テレメトリ出力内容
  1. テレメトリ番号　テレメトリ収集開始からの通し番号
  2. 起動からの経過時間（時：分：秒）
  3. 姿勢情報 Pitch
  4. 姿勢情報 Roll
  5. 姿勢情報 Yaw　（MPU6886からは取得不可）
  6. 加速度・ジャイロセンサ(MPU6886)内部温度

### (4) LED秒数ドット表示機能
* 25個のLEDを用い、0〜49秒を表します(setLedSecDotDisp)
* ここで言う秒は、起動からの経過時間を50で割った余りのことです
  * 0秒      : 全消灯
  * 1〜24秒  : 上から１行ずつ左から右へ１個ずつ点灯していく
  * 25秒     : 全点灯
  * 26〜49秒 : 上から１行ずつ左から右へ１個ずつ消灯していく
* 温度表示中は秒数ドット表示を行いません

### (5) LEDメッセージ温度表示機能
* 加速度・ジャイロセンサの内部温度をLEDマトリスクスにスクロール表示します(dispTemp)
* 温度カラーテーブル(temp_col_tbl)を用い、音頭によって表示する色カラーを変えることができます


