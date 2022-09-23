#include <ESP8266WiFi.h>//ESP8266基础联网库，包含大量联网协议功能和tcp操作
#include <WiFiUdp.h>

const char *ssid = "LiuXin";//联网WIFI名称
const char *password = "liuxin2020";//WIFI密码
unsigned int localPort=4210;
WiFiUDP Udp;

void setup() {
  Serial.begin(9600);//串口初始化
  WiFi.mode(WIFI_AP_STA);//混合联网模式
  WiFi.begin(ssid, password);//连接WIFI
  Serial.print("\nConnecting");
  while (WiFi.status() != WL_CONNECTED) {//是否连接成功
    delay(500);
    Serial.print(".");
  }
  Serial.print(WiFi.localIP());//打印获取到的IP
  Serial.println("==Success");
  WiFi.softAP("IotNet","12344321");//开启局域网WIFI来连接终端
  Serial.print("AP:IotNet;PW:12344321;IP:");
  Serial.println(WiFi.softAPIP());
  Udp.begin(localPort);
}

void Send(char ch[])
{
   Udp.beginPacket("255.255.255.255", localPort);
   Udp.write(ch);
   Udp.endPacket();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() == WL_CONNECTED)
  {
    
    int rssi=123;
    char chh[5];
    itoa(rssi,chh,10);
    Send(chh);
    Serial.println(chh);
    delay(2000);
    int packetSize = Udp.parsePacket();
    if (packetSize)//解析包不为空
    {
      Serial.printf("收到来自远程IP：%s（远程端口：%d）的数据包字节数：%d\n", Udp.remoteIP().toString().c_str(), Udp.remotePort(), packetSize);
      String udpStringVal = Udp.readString(); 
      Serial.print("开发板接收到UDP数据中的字符串 "); Serial.println(udpStringVal);
    }
  }
  delay(2000);
}
