#include "M5StickCPlus2.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <WiFiManager.h>

WiFiManager wifiManager;
WiFiManagerParameter custom_parameter_name("name", "Name", "HeartRate", 128);
WiFiManagerParameter custom_parameter_target_ip("TargetIP", "TargetIP", "127.0.0.1", 15);
WiFiManagerParameter custom_parameter_target_port("TargetPort", "TargetPort", "9000", 5);

char parameter_name[129];
char target_ip[16];
char target_port[6];

#define Display_BASE_X 0
#define Display_BASE_Y 24

// --- Wi-Fi 設定 ---
const char* ssid     = "AutoConnectAP";
const char* password = "*******";
const char* config_portal_name = "OSC APP";

// --- UDP / OSC ---
WiFiUDP    Udp;
IPAddress  targetIP;
uint16_t   targetPort;
const uint16_t localPort = 8001;


// --- Heart Rate Monitor ---
unsigned char counter;
unsigned long temp[21];
unsigned long sub;
bool data_effect=true;
unsigned int heart_rate;//the measurement result of heart rate

const int max_heartpluse_duty = 2000;//you can change it follow your system's request.
						//2000 meams 2 seconds. System return error 
						//if the duty overtrip 2 second.

void connectWiFiManager() {
  StickCP2.Display.clear();
  StickCP2.Display.setCursor(Display_BASE_X, 0);
  StickCP2.Display.print("starting config portal...");

  bool res;
  res = wifiManager.startConfigPortal(config_portal_name);
  if (!res) {
    Serial.println("failed to connect or hit timeout");
    delay(3000);
    // ESP.restart();
    StickCP2.Display.clear();
    StickCP2.Display.setCursor(Display_BASE_X, 0);
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    StickCP2.Display.print("WiFiManager connected");
  }

  // 拡張パラメータを取得
  strcpy(parameter_name, custom_parameter_name.getValue());
  strcpy(target_ip, custom_parameter_target_ip.getValue());
  strcpy(target_port, custom_parameter_target_port.getValue());

  // IPアドレスとポート番号を設定
  targetIP.fromString(target_ip);
  targetPort = atoi(target_port);
  Serial.printf("Target IP: %s, Port: %d\n", targetIP.toString().c_str(), targetPort);
}

void initWiFiManager() {
  wifiManager.setConfigPortalTimeout(360);
  wifiManager.addParameter(&custom_parameter_name);
  wifiManager.addParameter(&custom_parameter_target_ip);
  wifiManager.addParameter(&custom_parameter_target_port);

  bool res;
  res = wifiManager.autoConnect(ssid, password);
  if(!res) {
    connectWiFiManager();
  } else {
    Serial.println("Connected to WiFi");
    StickCP2.Display.clear();
    StickCP2.Display.setCursor(Display_BASE_X, 0);
    StickCP2.Display.print("WiFi OK");
    delay(1000);
  }
}

void arrayInit()
{
	for(unsigned char i=0;i < 20;i ++)
	{
		temp[i]=0;
	}
	temp[20]=millis();
}

// 心拍センサー
// https://github.com/Seeed-Studio/Grove_Ear_Clip_Heart_Rate_Sensor

void sum() {
  if(data_effect)
  {
    heart_rate=1200000/(temp[20]-temp[0]);//60*20*1000/20_total_time 
    Serial.print("Heart_rate_is:\t");
    Serial.println(heart_rate);
  }
  data_effect=1;//sign bit
}
static void heartRateInterrupt(void) {
  temp[counter]=millis();
	// Serial.println(counter,DEC);
  // Serial.println(temp[counter]);
  switch(counter)
	{
		case 0:
			sub=temp[counter]-temp[20];
			Serial.println(sub);
			break;
		default:
			sub=temp[counter]-temp[counter-1];
			// Serial.println(sub);
			break;
	}
  if(sub>max_heartpluse_duty)//set 2 seconds as max heart pluse duty
	{
		data_effect=0;//sign bit
		counter=0;
		// Serial.println("Heart rate measure error,test will restart!" );
		arrayInit();
	}
  if (counter==20&&data_effect)
  {
    counter=0;
    sum();
  }
  else if(counter!=20&&data_effect)
  {
    counter++;
  }
  else 
  {
    counter=0;
    data_effect=1;
  }
}

void initHeartRateSensor() {
  arrayInit();
  // G33 に 割り込み設定
  attachInterrupt(digitalPinToInterrupt(G33), heartRateInterrupt, RISING);
}

void setup() {
  Serial.begin(115200);

  // M5StickC Plus2 初期化
  auto cfg = M5.config();
  StickCP2.begin(cfg);

  // 画面設定
  StickCP2.Display.setRotation(1);
  StickCP2.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  StickCP2.Display.setTextSize(2);

  // Wi-Fi マネージャーの初期化
  initWiFiManager();

  // 心拍センサーの初期化
  initHeartRateSensor();

  Udp.begin(localPort);
  configTime(0,0,"pool.ntp.org");  // NTP 時刻同期
}

void loop() {
  M5.update();
  // Aボタン押下
  if (M5.BtnA.wasPressed()) {
    // Wi-Fi マネージャーを起動
    connectWiFiManager();
    return;
  }

  // IMU 更新＆生加速度取得
  if (StickCP2.Imu.update()) {
    // タイムスタンプ (ms since epoch)
    uint64_t ts = (uint64_t)time(NULL) * 1000 + (millis() % 1000);

    // OSC 送信
    OSCMessage msg("/avatar/parameters/HeartRate");
    msg.add((int32_t)(ts & 0xFFFFFFFF));
    msg.add(parameter_name);
    msg.add(heart_rate);
    Udp.beginPacket(targetIP, targetPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();

    // 画面に現在送信中の値を表示
    StickCP2.Display.clear();

    // パラメータを表示
    StickCP2.Display.setCursor(Display_BASE_X, Display_BASE_Y * 0);
    StickCP2.Display.printf("%s", parameter_name);    
    StickCP2.Display.setCursor(Display_BASE_X, Display_BASE_Y * 1);
    StickCP2.Display.printf("%s:%d", targetIP.toString().c_str(), targetPort);

    StickCP2.Display.setCursor(Display_BASE_X, Display_BASE_Y * 3);
    StickCP2.Display.printf("%d", heart_rate);
  }

  delay(50);  // ≈20Hz 更新
}