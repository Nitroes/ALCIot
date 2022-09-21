#include <ESP8266WiFi.h>
#include <Servo.h>
const char *ssid = "IotNet";//局域网WIFI名称
const char *password = "12344321";//WIFI密码
const char* Server = "192.168.4.1";//TCP服务器IP
const int Port = 23;//TCP服务器端口
short Status = 0; //连接服务器状态
int swPin = 5;
WiFiClient client;
Servo zi;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);//初始化串口
  WiFi.mode(WIFI_STA);//设置为STA模式
  WiFi.begin(ssid, password);//连接局域网
  Serial.print("\nConnecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print(WiFi.localIP());
  Serial.println("==Success");
  zi.attach(swPin);//设置舵机端口
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!Status) {
      client.connect(Server, Port);//连接TCP服务器
    }
    if (client.connected()) {
      if (!Status) {
        Serial.println("ConnectServerSuccess");
        Status = 1;
      }
      while (client.connected() || client.available()) {
        if (client.available()) {
          String line = client.readStringUntil('\n');//获取数据终端下发数据
          Serial.print("ReceiveCmd:");
          Serial.println(line);
          if (line.indexOf("led=1;") != -1) {//获取数据终端控制状态
            Serial.println("on");
            zi.write(120);//舵机转动角度
          } else if (line.indexOf("led=2;") != -1) { //获取数据终端控制状态
            Serial.println("off");
            zi.write(0);//舵机转动角度
          }
          delay(50);
        }
      }
    } else {
      Status = 0;
      client.stop();                      // 断开与服务器的连接
      Serial.print("Disconnected from:"); // 并且通过串口输出断开连接信息
      Serial.println(Server);
      delay(2000);
    }
  }
}
