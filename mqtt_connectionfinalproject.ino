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
#include <NTPClient.h>
#include <WiFiUdp.h>
//#include<time.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//#define DHTpin 2   //Set DHT pin as GPIO2
//DHTesp dht;

/**** LED Settings *******/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
const int led = LED_BUILTIN;
const long utcOffsetInSeconds = 19800; // UTC offset for Indian Standard Time (IST) in seconds (5 hours + 30 minutes)
//Set LED pin as GPIO5
/****** WiFi Connection Details *******/
const char* ssid = "PMP";
const char* password = "pmp@12345";
unsigned int globalCount;
unsigned int count;
bool publishMessageToMqtt = false;
int sensorPin = D5;
String machineSiteId = "SH-03";
//int lastResetDay = 0;
//unsigned long lastResetTime = 0; // Store the last time the counter was reset
//unsigned long resetInterval = 6 * 1000; // Reset interval: 24 hours in milliseconds
const char* machineTopic = "esp8266_data_SH-03";
int rsetPin=D6;
int i=0; 
int lastResetHour1 = -1; // Store the last reset hour for the first shift
int lastResetHour2 = -1; // Store the last reset hour for the second shift

// Define the hours at which you want to reset the counter (in 24-hour format)
const int resetHour1 = 8;  // Reset at 8:00 AM
const int resetHour2 = 20; // Reset at 8:00 PM
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
  String siteId = doc["siteId"];
  int count = doc["count"];

  // Use the extracted count value in your code
  if(machineSiteId==siteId){
    Serial.println("Count: " + String(count));
    globalCount = count;
  }
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
  
  timeClient.begin();
  timeClient.setTimeOffset(utcOffsetInSeconds);
  client.setCallback(callback);
  client.subscribe(machineTopic);
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print(machineSiteId);
 
}
void loop() {
   timeClient.update();
  
 // Get current day
  int currentDay = timeClient.getDay();
  // Get current hour
  int currentHour = timeClient.getHours();

  unsigned long epochTime = timeClient.getEpochTime();
  String formattedTime = timeClient.getFormattedTime();


  Serial.println("Epoch Time: " + String(epochTime));
  Serial.println("Formatted Time: " + formattedTime + "\n");
  lcd.setCursor(3,0);
  lcd.print(machineSiteId);
  

  // Use the real-time information for your tasks here

  //delay(1000);
  
  int sensorValue = digitalRead(sensorPin);
  int rset=digitalRead(rsetPin);
    // Check if a client has connected
  // WiFiClient client = server.available();
      if (!client.connected()) reconnect(); // check if client is connected
  
  client.loop();
  
  
if(count==0)
    count=globalCount;
    
 // if (millis() - lastResetTime >= resetInterval) {
    // Reset the counter to zero
   // count = 0;
    //globalCount=0;
    //lcd.clear();
    // Update the last reset time
    //lastResetTime = millis();
 // }
// if (currentDay != lastResetDay) {
//    count = 0; // Reset the counter to zero
//    globalCount=0;
//    lcd.clear();
//    lastResetDay = currentDay; // Update the last reset day
//    //Serial.println("Counter reset to zero.");
// }
 // Check if it's time to reset the counter for the first shift
  if (currentHour == resetHour1 && currentHour != lastResetHour1) {
    count = 0;
    globalCount=0;// Reset the counter to zero
    lcd.clear();
    lastResetHour1 = currentHour; // Update the last reset hour for the first shift
    Serial.println("Counter reset to zero for the first shift.");
  }

  // Check if it's time to reset the counter for the second shift
  if (currentHour == resetHour2 && currentHour != lastResetHour2) {
    count = 0;
    globalCount=0;// Reset the counter to zero
    lcd.clear();
    lastResetHour2 = currentHour; // Update the last reset hour for the second shift
    Serial.println("Counter reset to zero for the second shift.");
  }
 
    if(sensorValue==HIGH)
 {
  if(count==1)
  count=globalCount+1;
   count++;
    client.loop();
  
  // if(count==1)
  //  count=globalCount+count;
  publishMessageToMqtt = true;
   Serial.println(count);
  
    display.setCursor(0, 10);
   display.clearDisplay();
    display.setTextSize(2);
   display.println("machine100");
    display.setTextSize(2);
   display.setCursor(5,40);
   
display.println(count);
  display.display();  
// 
//    Serial.print("MACHINE 100 = "); Serial.println(count);
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
    //if(count==1)
    //count=globalCount+count;
    sensorValue = digitalRead(sensorPin);
    if(sensorValue==LOW)
        break;
    else
    {
    if (currentHour == resetHour1 && currentHour != lastResetHour1) {
    count = 0;
    globalCount=0;// Reset the counter to zero
    lcd.clear();
    lastResetHour1 = currentHour; // Update the last reset hour for the first shift
    Serial.println("Counter reset to zero for the first shift.");
  }

  // Check if it's time to reset the counter for the second shift
  if (currentHour == resetHour2 && currentHour != lastResetHour2) {
    count = 0;
    globalCount=0;// Reset the counter to zero
    lcd.clear();
    lastResetHour2 = currentHour; // Update the last reset hour for the second shift
    Serial.println("Counter reset to zero for the second shift.");
  }
//    {if (currentDay != lastResetDay) {
//    count = 0; // Reset the counter to zero
//    globalCount=0;
//    lcd.clear();
//    lastResetDay = currentDay; // Update the last reset day
//    //Serial.println("Counter reset to zero.");
// }  
     // if (millis() - lastResetTime >= resetInterval) {
    // Reset the counter to zero
    //count = 0;
   // globalCount=0;
    //lcd.clear();
    // Update the last reset time
    //lastResetTime = millis();
  }delay(5);
    
      goto again;
    }
  }

 
 lcd.setCursor(2,3);
 lcd.print(count);
//    Serial.print("MACHINE 100 count = "); Serial.println(count);
   
 if(count!=0 && count%10==0 && publishMessageToMqtt)
{


 DynamicJsonDocument doc(1024);

  doc["deviceId"] = "nodemcu";
  doc["siteId"] = machineSiteId;
  doc["count"] = count;
  doc["publishedTime"] = formattedTime;
  Serial.print(epochTime);

  char mqtt_message[400];
  serializeJson(doc, mqtt_message);

  publishMessage(machineTopic, mqtt_message, true);
  publishMessageToMqtt=false;
//  i=0;
}

//i++;
  delay(50);

}
