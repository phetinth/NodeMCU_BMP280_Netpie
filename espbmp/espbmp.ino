#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <ESP8266WiFi.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10

Adafruit_BMP280 bme; // I2C
const char* ssid     = "Redmi"; // ssid wifi
const char* password = "88888888"; // password wifi
WiFiClient client;


/* connect to netpie */
#include <MicroGear.h>
#define APPID "FeedBMP" // APPLICATION (APPID) etc. KMUTNBM2
#define KEY "Q22rjV6asSb8Eue" // device key แต่ในเว็บ netpie ใช้ session key
#define SECRET "kumaAPFihQK2KKVpAYQnViWxZ"  // *secret device key
#define FEEDID  "BMPFeed"  // feed name
#define ALIAS   "piebmp"
MicroGear microgear(client);

/* Variable */
int timer = 0;
char str[32];
float T;
float P;
float A;

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  msg[msglen] = '\0';
  Serial.println((char *)msg);
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.setAlias(ALIAS);
}

/**************************  
 *   S E T U P
 **************************/
void setup() {
  Serial.begin(9600);
  
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(CONNECTED, onConnected);     //optional
  microgear.on(ERROR, onConnected);
  microgear.on(INFO, onConnected);

Serial.println("Starting...");
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);

  
  Serial.println(("BMP280 test"));
  
  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
 
}

void loop() {

if (microgear.connected()) {
    Serial.print("*");
    microgear.loop();
    if (timer >= 3000) {
      T = bme.readTemperature();
      P = bme.readPressure();
      A = bme.readAltitude(1013.25);
      
      sprintf(str, "%d,%d,%d", T, P, A);
      Serial.println(str);
      String data = "{\"T\":";
      data += T ;
      data += ",\"P\":";
      data += P ;
      data += ",\"A\":";
      data += A ;
      data += "}";
      Serial.println((char*) data.c_str());
      if (isnan(T) || isnan(P) || isnan(A) || A >= 200000 || P >= 200000 || A >= 200000 ) {
        Serial.println("Failed to read from BMP sensor!");
      } else {
        Serial.print("Sending --> ");
        microgear.writeFeed(FEEDID, data);    //YOUR  FEED ID, API KEY
      }
      timer = 0;
    }
    else timer += 200;
  } else {
    Serial.println("connection lost, reconnect...");
    if (timer >= 5000) {
      microgear.connect(APPID);
      timer = 0;
    }
    else timer += 200;
}

  
    //Serial.print("T=");
    //Serial.print(bme.readTemperature());
    //Serial.print(" *C");
    
   // Serial.print(" P=");
  //  Serial.print(bme.readPressure());
  //  Serial.print(" Pa");

   // Serial.print(" A= ");
   // Serial.print(bme.readAltitude(1013.25)); // Standard Mean sea level Pressure
   // Serial.println(" m");

    
    //every 2 sec   
    delay(2000);
}
