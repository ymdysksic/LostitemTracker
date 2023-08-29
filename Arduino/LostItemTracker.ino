#include <Wire.h>
#include <SparkFun_MMA8452Q.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

MMA8452Q accel;
TinyGPSPlus gps;
SoftwareSerial ble(2, 3);
SoftwareSerial gpsSerial(6, 7); // RX, TX

const int SWITCH_PIN = 4;
int cnt = 0;

//自由落下判定の閾値
double Thresh_FF = 0.8; //自由落下判定する加速度の上限値
int Time_FF = 10;       //自由落下判定する時間間隔
int Time = 0;           //経過時間
int Delay = 10;         

double AccelXOld = 0;   //前ループのx方向加速度
double AccelYOld = 0;   //前ループのy方向加速度
double AccelZOld = 0;   //前ループのz方向加速度
double AccelXNow = 0;   //現ループのx方向加速度
double AccelYNow = 0;   //現ループのy方向加速度
double AccelZNow = 0;   //現ループのz方向加速度


void setup() {
  Serial.begin(9600);
  ble.begin(9600);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  Wire.begin();
  accel.begin();
  gpsSerial.begin(9600);
}

void loop() {

  //BLE接続関連
  int is_switch = digitalRead(SWITCH_PIN);

  if (ble.available() > 0) {
    String dataStr = ble.readStringUntil('\n');
    Serial.print("Receive:");
    Serial.println(dataStr);
  }

  if (is_switch == LOW) {

    ble.println("ECIO");

    Serial.println(cnt);
    ble.print("TXDT");
    ble.println(cnt);

    cnt++;
  }

  if (accel.available()) {
    Serial.print(accel.getCalculatedX(), 3);
    Serial.print("\t");
    Serial.print(accel.getCalculatedY(), 3);
    Serial.print("\t");
    Serial.print(accel.getCalculatedZ(), 3);
    Serial.println();

    //現在の加速度を保管
    AccelXNow = accel.getCalculatedX();
    AccelYNow = accel.getCalculatedY();
    AccelZNow = accel.getCalculatedZ();

    //現在の加速度が閾値以下になっているか確認
    if(AccelXNow < Thresh_FF && AccelYNow < Thresh_FF && AccelZNow < Thresh_FF){
      
      //前ループの加速度も閾値以下だったかどうか確認
      if(AccelXOld < Thresh_FF && AccelYOld < Thresh_FF && AccelZOld < Thresh_FF){

        //前ループの加速度が閾値以下かつ現ループの加速度も閾値以下だった場合、経過時間を加算
        Time = Time + Delay;

        //経過時間が閾値を超えた場合、落下判定とする
        if(Time > Time_FF){
          Serial.println("落下検知");

          //位置情報をスマホに送信する
          NoticeLocation();
          
        }
      }
      //前ループの加速度が閾値以上＝落下していないとして、経過時間をリセットする
      else{
        Time = 0;
      }
    }

    //今ループの加速度を前ループの加速度として保存
    AccelXOld = AccelXNow;
    AccelYOld = AccelYNow;
    AccelZOld = AccelZNow;

    //スマホから位置情報の要求が来た場合も送信する
    //Todo:処理を実装する

  }
  delay(Delay);
}

void NoticeLocation(){

  //Todo:位置情報をスマホに送る処理を実装する
  
  while (gpsSerial.available() > 0){
    char c = gpsSerial.read();
    gps.encode(c);

    // 位置情報が更新されたか
    if (gps.location.isValid()){
        // 緯度・経度・標高を取得
        Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
        Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
        Serial.print("ALT=");  Serial.println(gps.altitude.meters());
    }
  }
}