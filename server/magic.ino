#include <ESP8266WiFi.h>//ESP8266基础联网库，包含大量联网协议功能和tcp操作
#include <ESP8266HTTPClient.h>//用于HTTP协议通讯利用互联网或局域网向网络服务器发送HTTP请求，并且分析网络服务器返回的HTTP响应信息，从而实现物联网应用。
#include <ArduinoJson.h>//进行JSON操作和数据转换
#define MAX_SRV_CLIENTS 2 //最大连接终端数量

const char *ssid = "CAP";//联网WIFI名称
const char *password = "13119559439";//WIFI密码
const char *address = "http://192.168.1.108:8080/api/Cloud";//服务器地址
const unsigned long HTTP_TIMEOUT = 5000;//服务器超时时间
WiFiClient client, serverClients[MAX_SRV_CLIENTS];//最大连接终端数量
WiFiServer server(23);//TCP服务器和定义端口
HTTPClient http;//http工具对象
String response,temp,hum,lx;//储存变量

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
  server.begin();//TCP服务启动
  server.setNoDelay(true);//关闭小包合并
  WiFi.softAP("IotNet","12344321");//开启局域网WIFI来连接终端
  Serial.print("AP:IotNet;PW:12344321;IP:");
  Serial.println(WiFi.softAPIP());
}

bool GetHttp()
{
  bool r = false;
  http.setTimeout(HTTP_TIMEOUT);//超时设置
  WiFiClient client;
  HTTPClient http;//http工具对象
  http.begin(client, address);//连接服务器
  int httpCode = http.GET();//GET操作
  if (httpCode > 0)
  {
    //判断是否获取到
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
    {
      response = http.getString();//存入全局变量
      Serial.print("HttpGet:");
      Serial.println(response);//打印GET到的数据
      r = true;
    }
  }
  http.end();//结束连接
  return r;//获取失败则为false 成功为true
}

void PostHttp(String msg)
{
  http.setTimeout(HTTP_TIMEOUT);//超时设置
  WiFiClient client;
  HTTPClient http;//http工具对象
  http.begin(client, address);//连接服务器
  //声明发送数据的类型
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(msg);//POST操作发送数据msg
  Serial.print("PostHttp:");
  Serial.println(msg);//打印发送的数据
  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      Serial.print("PostHttp_Res:");
      Serial.println(http.getString());//打印返回的数据
    }
  }
  http.end();//结束连接
  delay(50);
}

void ThreadClient() {
  uint8_t i, c = 0;
  //检测是否有新的client请求进来
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      //释放旧无效或者断开的client
      if (!serverClients[i] || !serverClients[i].connected()) {//终端是否还在连接
        if (serverClients[i]) {
          serverClients[i].stop();//中断终端连接
        }
        //分配最新的client
        serverClients[i] = server.available();
        Serial.print("NewClient: ");
        Serial.println(i);
        break;
      }
    }
  }
  //当达到最大连接数 无法释放无效的client，需要拒绝连接
  if (i == MAX_SRV_CLIENTS) {
    WiFiClient serverClient = server.available();
    serverClient.stop();
    Serial.println("Connection rejected");
  }
  //上传在线设备数量
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i].connected()) {
      c++;
    }
  }
  String msg="onLineClient="+String(c)+"&temp="+String(temp)+"&hum="+String(hum)+"&lx="+String(lx);
  PostHttp(msg);//发送连接终端的数量还有采集到的数据到服务器
}

void ThreadSendCmd() {
  uint8_t i;
  if (GetHttp())//获取服务器数据
  {
    const size_t capacity = 1024;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, response);//转为JSON
    String msg=doc["led"];//获取控制状态
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i] && serverClients[i].connected()) {
        serverClients[i].println("led="+msg+";");//发送控制状态到终端
        Serial.println("SendCmd:led="+msg+";");
        delay(50);
      }
    }
  }
}

void ThreadReceiveCmd() {
  uint8_t i;
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available()) {
        //get data from the telnet client and push it to the UART
        while (serverClients[i].available()) {
          
          String cmd = serverClients[i].readStringUntil('\n');//通过换行符分隔开TCP数据包
          Serial.print("ReceiveCmd:");
          Serial.println(cmd);//打印数据
          int tIndex=cmd.indexOf("temp=")+5;
          temp=cmd.substring(tIndex,tIndex+4);//获取温度
          int hIndex=cmd.indexOf("hum=")+4;
          hum=cmd.substring(hIndex,hIndex+4);//获取湿度
          int lxIndex=cmd.indexOf("lx=")+3;
          lx=cmd.substring(lxIndex,cmd.indexOf("\r"));//获取光照
        }
      }
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() == WL_CONNECTED)
  {
    ThreadClient();
    ThreadSendCmd();
    ThreadReceiveCmd();
  }
  delay(2000);
}
