#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <ESP.h>
#include "esp_deep_sleep.h"
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define UPDATE_TIME_DELAY 1000

//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
#define uS_TO_S_FACTOR 1000000

unsigned long now;
int DEEPSLEEP_SECONDS = 1800;

uint64_t chipid;

long timeout;

const int dhtpin = 5;
const int soilpin = 25;
const int POWER_PIN = 32;
const int LIGHT_PIN = 33;

// Initialize DHT sensor.
DHT dht(dhtpin, DHTTYPE);
BluetoothSerial SerialBT;

// Temporary variables
static char celsiusTemp[7];
static char humidityTemp[7];

// Client variables 
char linebuf[80];
int charcount=0;

char deviceid[21];

void setup() {
  dht.begin();
  
  Serial.begin(115200);
  SerialBT.begin("Flower Care0"); //Bluetooth device name

  pinMode(16, OUTPUT); 
  pinMode(POWER_PIN, INPUT);
  digitalWrite(16, LOW);  

  timeout = 0;

  chipid = ESP.getEfuseMac();
  sprintf(deviceid, "%" PRIu64, chipid);
  Serial.print("DeviceId: ");
  Serial.println(deviceid);
}

void loop() {
  
  char body[100];
  digitalWrite(16, LOW); //switched on

  sensorsData(body);
  Serial.println(body);
  SerialBT.write((const uint8_t *)body, strlen(body));

  delay(UPDATE_TIME_DELAY);
}

void sensorsData(char* body){

  //This section read sensors
  timeout = millis();
  
  int waterlevel = analogRead(soilpin);
 // int lightlevel = analogRead(LIGHT_PIN);
  
  waterlevel = map(waterlevel, 3400,2000 , 0, 1023);
  waterlevel = constrain(waterlevel, 0,1023 );
  
  waterlevel = waterlevel;
  waterlevel = constrain(waterlevel, 0, 1023);
  waterlevel = map(waterlevel, 0, 1023, 0, 100);
  
  //lightlevel = map(lightlevel, 0, 4095, 0, 1023);
  //lightlevel = constrain(lightlevel, 0, 1023);
  
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();
  
  float hic = dht.computeHeatIndex(temperature, humidity, false);       
  dtostrf(hic, 6, 2, celsiusTemp);               
  dtostrf(humidity, 6, 2, humidityTemp);
  
  //String did = String(deviceid);
  String water = String((int)waterlevel);
  //String light = String((int)lightlevel);
  //strcpy(body, "deviceId:");
  //strcat(body, did.c_str());
  //strcat(body, ",water:");
  strcpy(body, "water:");
  strcat(body, water.c_str());
  //strcat(body, ",light:");
  //strcat(body, light.c_str());
  strcat(body, ",humidity:");
  strcat(body, humidityTemp);
  strcat(body, ",temperature:");
  strcat(body, celsiusTemp);
  strcat(body, "\n\r");
}
