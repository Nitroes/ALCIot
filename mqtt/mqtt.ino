#include <ESP8266WiFi.h>//ESP8266基础联网库，包含大量联网协议功能和tcp操作
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID       "LiuXin"
#define WLAN_PASS       "liuxin2020"

#define AIO_SERVER      "www.aliceproject.top"
#define AIO_SERVERPORT  21883                   // use 8883 for SSL
#define AIO_USERNAME    "iot3"
#define AIO_KEY         ""

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt,"iot/senser");
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt,"iot");

void MQTT_connect();

void setup() {
  Serial.begin(9600);//串口初始化
  WiFi.mode(WIFI_AP_STA);//混合联网模式
  WiFi.begin(WLAN_SSID, WLAN_PASS);//连接WIFI
  Serial.print("\nConnecting");
  while (WiFi.status() != WL_CONNECTED) {//是否连接成功
    delay(500);
    Serial.print(".");
  }
  Serial.print(WiFi.localIP());//打印获取到的IP
  Serial.println("==Success");
  Serial.println(WiFi.softAPIP());
  mqtt.subscribe(&onoffbutton);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() == WL_CONNECTED)
  {
    MQTT_connect();
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000))) {
      if (subscription == &onoffbutton) {
        Serial.print(F("Got: "));
        Serial.println((char *)onoffbutton.lastread);
      }
    }
  }
  delay(2000);
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
