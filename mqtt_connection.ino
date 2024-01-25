#ifdef ESP8266
 #include <ESP8266WiFi.h>
 #else
 #include <WiFi.h>
#endif

//#include "DHTesp.h"
#include <ArduinoJson.h>
#include<SPI.h>
#include<Wire.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LiquidCrystal_I2C.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//#define DHTpin 2   //Set DHT pin as GPIO2
//DHTesp dht;

/**** LED Settings *******/
const int led = LED_BUILTIN; //Set LED pin as GPIO5
/****** WiFi Connection Details *******/
const char* ssid = "Airtel_vipu_4970";
const char* password = "Password@987";
unsigned int globalCount;
unsigned int count;
int sensorPin = D5;
int i=0; 
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
/******* MQTT Broker Connection Details *******/
const char* mqtt_server = " d08900cfdef5463098201f44a1532917.s2.eu.hivemq.cloud";
const char* mqtt_username = "ankit";
const char* mqtt_password = "Ankit@123";
const int mqtt_port =8883;
WiFiClientSecure espClient;

/**** MQTT Client Initialisation Using WiFi Connection *****/
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
static const char *root_ca PROGMEM = R"EOF(

-----BEGIN CERTIFICATE-----
MIIFnjCCA4YCCQDm7/FV5GARXjANBgkqhkiG9w0BAQ0FADCBqTELMAkGA1UEBhMC
REUxEDAOBgNVBAgMB0JhdmFyaWExETAPBgNVBAcMCEVybGFuZ2VuMRcwFQYDVQQK
DA5Tb2Z0YmxhZGUgR21iSDEZMBcGA1UECwwQd3d3LnNvZnRibGFkZS5kZTEcMBoG
A1UEAwwTU29mdGJsYWRlIFJvb3QgQ0EgMTEjMCEGCSqGSIb3DQEJARYUY29udGFj
dEBzb2Z0YmxhZGUuZGUwHhcNMjMxMjIwMTkxNjE5WhcNMjQwNDE4MTkxNjE5WjB4
MQswCQYDVQQGEwJERTEMMAoGA1UECAwDTlJXMREwDwYDVQQHDAhFcmxhbmdlbjES
MBAGA1UECgwJU29mdGJsYWRlMRIwEAYDVQQDDAlTb2Z0YmxhZGUxIDAeBgkqhkiG
9w0BCQEWEW5pa29Ac29mdGJsYWRlLmRlMIICIjANBgkqhkiG9w0BAQEFAAOCAg8A
MIICCgKCAgEAqUE9p71j0zzCIQJlacOrA/k7EvNO98jdVUS77opS2ONUg6mwWZ8H
QJrJbce00xN8RPTPfKG4gmpWYY9JEIl9U4baYGAjMALiEaInxODZ8TF1IqyKdmyL
9Q7t+Fd+goGZy1rZUeLDxU40OXZ0okJu1vc5lrXkVDQtscPeEvB/HORsV9+9LM0v
hUIx+S+SmF0CCh/oNWlmIW5lL0kViD49YC8jaULLzEgljh4rV3rQwKp40MBmFJkt
E3eMa3vfjHwDTlsTY84gLhAY5gYPzlTJPCXFrMlmiSGc5/tl3TTyT1+TJAEP3pGJ
ktRbEmlQi5FwOaGgSfbUj+5Sq1t+ZLp0stmB2wYh0izN/zklTB9FJMN93o/dbqVm
TS4ygxiUD3wuxL/yQryMoSHkPgZQfEEd4fHc22eetCrBwJRgs4wP528WfZ/Kup8o
SpAXexJlptFfiLY00XzXNxDWfzwEKBbJ5JUnNRIvzZ2Pul1ZYZ+/FKjjLUG7hmBW
nkkmUcu3758bbCRccwqiIlLcQrzx48TnOkiIpEoIEPKpBFkwpURLYMyrMfK9CSFO
l28/xYqf7WNdywABCmqCIiQIyvoIHDvYF3HR3fcEJ/4kqAA+Zt0A6X3uK2Bvsr9h
LuwaZnKPC2r3ghvqKIvTzvztj7isL4RRTrkf/0hZ/Xg5MIxsZCsRZ3cCAwEAATAN
BgkqhkiG9w0BAQ0FAAOCAgEAyOS0FlPaefn8jzytyxl45DR2gKz9jvqAVD8X+IzN
Z7aHtAmf5wPG9Jzb/WpJLOLfAnRPat+W7/SvAUOoMSXbiqN0s7o34nLMYne6Fumd
XHoRvo3B2s18IC1mpoz1KFgWNKtd8bI3aZTbXOL9ch+6b/MxFROp7msjFxZ5sFcC
LiFZmmgG6vwmRdf+Wml9tGRcOKaSM7TvftJQjCyyE1EvefFkStK1Ss7ArZxuqyvA
XMBcna4vnW/1ibezOxcbXEpmq+DbisoJtXTnNubdfQ2QN8V9irZgDflcyIqZ+ZHu
2qsV4whGeQXoHdU0DojmD2NCVjtDna0xpo8bKotov60CLZ50NqBwx4Y9Oq6C4/9P
4zWrIf0MzlEO6zXogdlBAiLU3maHb1cvoDnJG4vIztOXl2kTfVMnzA27YT5jLOLw
5H0R9tfXEOnxy87ZCI9V0YMnO9p6ix1Zdp0x8otS6cOFgG4S7wI98HzRO2aeH9ut
hc7+zY+RLJDrAF6CCPt6K9mFza8bSgP9wzdWdl3cmIzhWpUYExQy8e7RtAxHLn5b
4rmI769UxEEBsxl+M3YyixMsIfNuKoQZTuyRz5vmMK7edbf0UUlaLv6Cp0q5aB7E
5llnjp/lo+f0sBxosZkJ9csId4wwRrKnqlMiDxQiI0OYqHJEcTRLJRfxYbunVxVP
ekU=
-----END CERTIFICATE-----
)EOF";
void setup_wifi() {
  delay(10);
  Serial.print("\nConnecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      client.subscribe("led_state");   // subscribe the topics here

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);

  //--- check the incomming message
//    if( strcmp(topic,"led_state") == 0){
//     if (incommingMessage.equals("1")) digitalWrite(led, HIGH);   // Turn the LED on
//     else digitalWrite(led, LOW);  // Turn the LED off
//  }
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, incommingMessage);
  int count = doc["count"];
  

  // Use the extracted count value in your code
  Serial.println("Count: " + String(count));
  globalCount = count;
}
void publishMessage(const char* topic, String payload , boolean retained){
  if (client.publish(topic, payload.c_str(), true))
      Serial.println("Message publised ["+String(topic)+"]: "+payload);
}
void setup() {
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
    
  }
   
  delay(55);
  display.clearDisplay();

 // display.setTextSize(2);
  display.setTextColor(WHITE);
 // dht.setup(DHTpin, DHTesp::DHT11); //Set up DHT11 sensor
  pinMode(led, OUTPUT); //set up LED
  Serial.begin(115200);
  while (!Serial) delay(1);
  setup_wifi();

  #ifdef ESP8266
    espClient.setInsecure();
  #else
    espClient.setCACert(root_ca);      // enable this line and the the "certificate" code for secure connection
  #endif

  client.setServer(mqtt_server, mqtt_port);
  reconnect();
  client.setCallback(callback);
  client.subscribe("esp8266_data_SH2");
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("SH-02");
}
void loop() {
  
  int sensorValue = digitalRead(sensorPin);
    // Check if a client has connected
  // WiFiClient client = server.available();
      if (!client.connected()) reconnect(); // check if client is connected
  
  client.loop();
  if(count==0)
    count = globalCount;
 
    if(sensorValue==HIGH)
 {
  count++;
   Serial.println(count);
  
    display.setCursor(0, 10);
   display.clearDisplay();
    display.setTextSize(2);
   display.println("machine100");
    display.setTextSize(2);
   display.setCursor(5,40);
   
display.println(count);
  display.display();  
 
    Serial.print("MACHINE 100 = "); Serial.println(count);
    display.display(); 
   display.setCursor(0, 10);
   display.clearDisplay();
    display.setTextSize(2);
   display.println("machine100");
    display.setTextSize(2);
   display.setCursor(5,40);
   
    display.println(count);
  display.display(); 
   again:
  while(sensorValue==HIGH) {
    sensorValue = digitalRead(sensorPin);
    if(sensorValue==LOW)
        break;
    else
    {  delay(5);
      goto again;
    }
  }
 }
 lcd.setCursor(2,3);
  lcd.print(count);
    Serial.print("MACHINE 100 count = "); Serial.println(count);
   
// if(i==10)
//{


 DynamicJsonDocument doc(1024);

  doc["deviceId"] = "SH-02";
  doc["siteId"] = "PMP";
  doc["count"] = count;

  char mqtt_message[128];
  serializeJson(doc, mqtt_message);

  publishMessage("esp8266_data_SH2", mqtt_message, true);
//  i=0;
//}
//i++;
  delay(50);

}
