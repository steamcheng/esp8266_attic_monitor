/* Attic monitor: 18 Jul 2016 

  This controls an ESP8266 in the garage which monitors 
  temperature and humidity and publishes data back to the MQTT broker.
  
*/

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DHT.h>

/************************* DHT22 config *********************************/

#define DHTTYPE DHT22   // DHT Shield uses DHT 22 sensor
#define DHTPIN D4       // DHT Shield uses pin D4

//Declare variables:
unsigned long lasttime;
char message_buff[100];
char envmessage_buff[100];
int ctr = 0;

// Create a DHT class
DHT dht(DHTPIN, DHTTYPE);

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "xxxx"
#define WLAN_PASS       "xxxx"

/************************* PubSub Setup *********************************/
#define MQTT_SERVER "192.168.1.5"

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient wlClient;
PubSubClient client(MQTT_SERVER, 1883, wlClient);

/*********************** Connect to MQTT server ***********************/
void mqtt_connect(){
    // Loop until we're reconnected
  while (!client.connected()) {
     Serial.print("Attempting MQTT connection...");
   //  Attempt to connect
    if (client.connect("AtticClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("openhab/atticEnvironment/status","Attic is alive!\0");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {

  Serial.begin(115200); //start serial commms for debugging
  delay(10);
  
  dht.begin();  // Initialize DHT22
  
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
//    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  mqtt_connect();
  Serial.println(F("Attic Monitor Online!"));
} // End setup()

void loop()
{

// Reconnect of connection lost.
   if (!client.connected()) {
    mqtt_connect();
   }
     
// Publish temperature and humidity every 15 seconds
   if (millis() > (lasttime + 15000)) {
    lasttime = millis();
    pubTempHum();
   }

   client.loop();
}  // End main loop()


void pubTempHum() {
   // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float f = dht.readTemperature(true);    // Read temperature as Fahrenheit (isFahrenheit = true)
   
  if (isnan(f) || isnan(h)) {
    Serial.println("Failed to read from DHT");
	client.publish("openhab/atticEnvironment/status","Attic is NaN!\0");
//    ctr = ctr+1;  // This is for a sensor restart function that needs to be added.
// Add DHT restart function here once hardware node is built to include this capability.
  }
  else{
	client.publish("openhab/atticEnvironment/status","Attic is alive!\0");
  }
// Publish temp and humidity here
   String fpubString = String(f);
   fpubString.toCharArray(envmessage_buff, fpubString.length()+1);
   Serial.print(F("\nSending DHT temperature ")); Serial.print(f);
   client.publish("openhab/atticEnvironment/temp", envmessage_buff);
   Serial.print(F("\nSending DHT humidity "));  Serial.println(h);
   String hpubString = String(h); 
   hpubString.toCharArray(envmessage_buff, hpubString.length()+1);  
   client.publish("openhab/atticEnvironment/humidity", envmessage_buff);
}  // End pubTempHum()
