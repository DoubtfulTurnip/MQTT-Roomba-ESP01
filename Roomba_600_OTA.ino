//This version is for the Roomba 600 Series
//Connect a wire from D4 on the nodeMCU to the BRC pin on the roomba to prevent sleep mode.

//Doubtful Note: I believe that D4 is the RX pin (blue) on the roomba IO but I am still testing.

// Roomba
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SimpleTimer.h>
#include <Roomba.h>
// AsyncElegant OTA
#include <Arduino.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>


//USER CONFIGURED SECTION START//
const char* ssid = "YOUR_WIRELESS_SSID";
const char* password = "YOUR_WIRELESS_SSID";
const char* mqtt_server = "YOUR_MQTT_SERVER_ADDRESS";
const int mqtt_port = YOUR_MQTT_SERVER_PORT;
const char *mqtt_user = "YOUR_MQTT_USERNAME";
const char *mqtt_pass = "YOUR_MQTT_PASSWORD";
const char *mqtt_client_name = "Roomba"; // Client connections can't have the same connection name
//USER CONFIGURED SECTION END//



// AsyncElegant OTA
AsyncWebServer server(80);

//Roomba
WiFiClient espClient;
PubSubClient client(espClient);
SimpleTimer timer;
Roomba roomba(&Serial, Roomba::Baud115200);


// Variables
bool toggle = true;
const int noSleepPin = 2;
bool boot = true;
long battery_Current_mAh = 0;
long battery_Voltage = 0;
long battery_Tl_mAh = 0;
long battery_percent = 0;
char battery_percent_send[50];
char battery_Current_mAh_send[50];
uint8_t tempBuf[10];

//Functions

void setup_wifi() 
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
  }
}

void reconnect() 
{
  // Loop until we're reconnected
  int retries = 0;
  while (!client.connected()) 
  {
    if(retries < 50)
    {
      // Attempt to connect
      if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass, "roomba/status", 0, 0, "Dead Somewhere")) 
      {
        // Once connected, publish an announcement...
        if(boot == false)
        {
          client.publish("checkIn/roomba", "Reconnected"); 
        }
        if(boot == true)
        {
          client.publish("checkIn/roomba", "Rebooted");
          boot = false;
        }
        // ... and resubscribe
        client.subscribe("roomba/commands");
      } 
      else 
      {
        retries++;
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    if(retries >= 50)
    {
    ESP.restart();
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  String newTopic = topic;
  payload[length] = '\0';
  String newPayload = String((char *)payload);
  if (newTopic == "roomba/commands") 
  {
    if (newPayload == "start")
    {
      startCleaning();
    }
    if (newPayload == "stop")
    {
      stopMoving();
    }
    if (newPayload == "return")
    {
      stopReturn();
    }
  }
}


void startCleaning()
{
  Serial.write(128);
  delay(50);
  Serial.write(131);
  delay(50);
  Serial.write(135);
  client.publish("roomba/status", "Cleaning");
}

void stopReturn()
{
  Serial.write(128);
  delay(50);
  Serial.write(131);
  delay(50);
  Serial.write(143);
  client.publish("roomba/status", "Returning");
}

void stopMoving()
{
  Serial.write(128);
  delay(50);
  Serial.write(131);
  delay(50);
  Serial.write(133);
  client.publish("roomba/status", "Stopped");
}

void sendInfoRoomba()
{
  roomba.start(); 
  roomba.getSensors(21, tempBuf, 1);
  battery_Voltage = tempBuf[0];
  delay(50);
  roomba.getSensors(25, tempBuf, 2);
  battery_Current_mAh = tempBuf[1]+256*tempBuf[0];
  delay(50);
  roomba.getSensors(26, tempBuf, 2);
  battery_Tl_mAh = tempBuf[1]+256*tempBuf[0];
  if(battery_Tl_mAh != 0)
  {
    int nBatPcent = 100*battery_Current_mAh/battery_Tl_mAh;
    String temp_str2 = String(nBatPcent);
    temp_str2.toCharArray(battery_percent_send, temp_str2.length() + 1); //packaging up the data to publish to mqtt
    client.publish("roomba/battery", battery_percent_send);
  }
  if(battery_Tl_mAh == 0)
  {  
    client.publish("roomba/battery", "NO DATA");
  }
  String temp_str = String(battery_Voltage);
  temp_str.toCharArray(battery_Current_mAh_send, temp_str.length() + 1); //packaging up the data to publish to mqtt
  client.publish("roomba/charging", battery_Current_mAh_send);
}

void stayAwakeLow()
{
  digitalWrite(noSleepPin, LOW);
  timer.setTimeout(1000, stayAwakeHigh);
}

void stayAwakeHigh()
{
  digitalWrite(noSleepPin, HIGH);
}



void setup() 
{
  //Roomba 
  pinMode(noSleepPin, OUTPUT);
  digitalWrite(noSleepPin, HIGH);
  Serial.begin(115200);
  Serial.write(129);
  delay(50);
  Serial.write(11);
  delay(50);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  timer.setInterval(5000, sendInfoRoomba);
  timer.setInterval(60000, stayAwakeLow);
  // AsyncElegant OTA Addition
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am a Roomba.");
  });
  // Change USERNAME and PASSWORD to preference
  AsyncElegantOTA.begin(&server, "USERNAME", "PASSWORD");
  // You can change the ID to whatever you want
  AsyncElegantOTA.setID("Roomba");
  server.begin();
  Serial.println("HTTP server started");
}

void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();
  timer.run();
}

