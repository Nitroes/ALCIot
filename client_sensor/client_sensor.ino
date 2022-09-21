#include <DHT.h>
#include <ESP8266WiFi.h>

const char *ssid = "IotNet";//局域网WIFI名称
const char *password = "12344321";//WIFI密码
const char* Server = "192.168.4.1";//TCP服务器IP
const int Port = 23;//TCP服务器端口
short Status = 0; //连接服务器状态
WiFiClient client;
DHT dht(5, DHT11); //温度传感器GPIO端口和传感器类型设置
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
  dht.begin();//温度传感器初始化
  delay(2000);//温度预热
  Serial.println("DHT11==Success");
  pinMode(A0, INPUT);//设置ADC端口为输入
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
      float t = dht.readTemperature();//采集温度
      float h = dht.readHumidity();//采集湿度
      int lx = 1024 - analogRead(A0);//采集光照 最大1024 越亮值越小
      String cmd = "temp=" + String(t, 1) + ";hum=" + String(h, 1) + ";lx=" + String(lx);
      client.println(cmd);//发送数据到数据终端
      Serial.print("SendCmd:");
      Serial.println(cmd);//打印发送的数据
      delay(3000);
    } else {
      Status = 0;
      client.stop();                      // 断开与服务器的连接
      Serial.print("Disconnected from:"); // 并且通过串口输出断开连接信息
      Serial.println(Server);
      delay(2000);
    }
  }
}
