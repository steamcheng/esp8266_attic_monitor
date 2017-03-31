/* Attic monitor: 18 Jul 2016 

  This controls a Wemos Mini D1 ESP8266 in the attic which monitors 
  temperature, humidity and attic fan status.
  
  Updated 20 Mar 2017 - added current sensor to check if attic fan is running.
  Also added additional status messages.

*/
#include <EmonLib.h>        	// Library containing the current calculations
#include <PubSubClient.h>	// MQTT pub/sub library
#include <ESP8266WiFi.h>	// Needed for WiFi
#include <WiFiClient.h>		// WiFi client library
#include <DHT.h>		// DHT temp and humidity sensor library

boolean debug = false;  // Set true for serial output for debugging

/************************* DHT22 config *********************************/

#define DHTTYPE DHT22   // DHT is type DHT 22 sensor.  Feel free to use a DHT 11, but change this to match.
#define DHTPIN D4       // DHT sensor uses pin D4 for data
#define DHTPWR D8       // DHT power pin.  Allows sketch to reset power if DHT hangs.

//Declare variables:
unsigned long lasttime;
char message_buff[100];
char envmessage_buff[100];
int ctr = 0;

// Create instances of DHT and EnergyMonitor
DHT dht(DHTPIN, DHTTYPE);
EnergyMonitor emon1;

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "xxxxx"
#define WLAN_PASS       "xxxxxxxxx"

/************************* PubSub Setup *********************************/
#define MQTT_SERVER "192.168.1.5"	// You'll also need to make sure this points to your broker

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient wlClient;
PubSubClient client(MQTT_SERVER, 1883, wlClient);

// Set everything up.
void setup() {

  pinMode(DHTPWR, OUTPUT); // initialize the DHTPWR pin as an output.
  pinMode(DHTPIN, INPUT); // initialize the DHT pin as an input.
  digitalWrite(DHTPWR,HIGH); // Turn on DHT power.
  pinMode(A0, INPUT);  // Analog pin for reading fan status - input.
  
  emon1.current(A0,15);  // Start up the current monitor using A0 and a correction of 15.
  
 if (debug) { Serial.begin(115200); } //start serial commms for debugging
  delay(10);
  
  dht.begin();  // Initialize DHT22
  
  // Connect to WiFi access point.
  if (debug) { Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID); }
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
//   if (debug) { Serial.print("."); }
  }
  if (debug) { Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP()); }
	
  // Now connect to the mqtt broker	
  mqtt_connect();
  if (debug) { Serial.println(F("Attic Monitor Online!")); }

} // End setup()

/* Main execution loop here */
void loop()
{

// Reconnect to mqtt broker if connection lost.
   if (!client.connected()) {
    mqtt_connect();
  }
 
// Publish temperature, humidity and fan status every 15 seconds
  if (millis() > (lasttime + 15000)) {
    lasttime = millis();
    pubTempHum();
    pubFanStatus();
  }
 client.loop();
} // End main loop

/*********************** Connect to MQTT server ***********************/
void mqtt_connect(){
    // Loop until we're reconnected
  while (!client.connected()) {
     if (debug) { Serial.print("Attempting MQTT connection..."); }
   //  Attempt to connect
    if (client.connect("AtticClient")) {
      if (debug) { Serial.println("connected"); }
      // Once connected, publish an announcement...
      client.publish("openhab/atticEnvironment/status","Attic is alive!\0");
    } else {
      if (debug) { Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds"); }
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} // End mqtt_connect()

/*********************** Read DHT sensor and publish readings ***********************/
 void pubTempHum() {
 
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
 
// Determine if the result is a valid numerical value  
 if (isnan(f) || isnan(h)) {
    if (debug) { Serial.println("Failed to read from DHT22"); }
	client.publish("openhab/atticEnvironment/status","DHT Failure!\0");
        client.publish("openhab/atticEnvironment/status/DHT","NaN!\0");
    ctr = ctr+1; // Counter increment if NaN
}
else{
	client.publish("openhab/atticEnvironment/status","Attic is alive!\0");
        client.publish("openhab/atticEnvironment/status/DHT","DHT is UP!\0");
}

// Here is where the DHT sensor gets restarted if the sensor hangs up. 
 if (ctr == 3){
   if (debug) { Serial.println("Restarting DHT1"); }
   client.publish("openhab/atticEnvironment/status/DHT","Restarting DHT sensor!\0");
   digitalWrite(DHTPWR,LOW);
   delay(250);
   digitalWrite(DHTPWR,HIGH);
   dht.begin();
   ctr = 0;
   client.publish("openhab/atticEnvironment/status/DHT","DHT restarted!\0");
 }
 
// Temp and humidity stuff here
   String fpubString = String(f);
   fpubString.toCharArray(envmessage_buff, fpubString.length()+1);
   if (debug) { Serial.print(F("\nSending DHT temperature ")); Serial.print(f); }
   client.publish("openhab/atticEnvironment/temp", envmessage_buff);
   if (debug) { Serial.print(F("\nSending DHT humidity "));  Serial.println(h); }
  String hpubString = String(h); 
  hpubString.toCharArray(envmessage_buff, hpubString.length()+1);  
   client.publish("openhab/atticEnvironment/humidity", envmessage_buff);
 
  } // End pubTempHum()
  
  // ************************** Determine fan running status here ******************* 
  void pubFanStatus() {
    double Irms = emon1.calcIrms(1500);
    String ipubString = String(Irms);
    ipubString.toCharArray(envmessage_buff, ipubString.length()+1);
  if (Irms > 1){
  if (debug) { Serial.print("Fan On \n");
    Serial.println(Irms); }
  client.publish("openhab/atticEnvironment/status/fan","Fan On\0");
  client.publish("openhab/atticEnvironment/amps", envmessage_buff);
  }
  else {
  if (debug) { Serial.print("Fan Off\n");
    Serial.println(Irms);  }
  client.publish("openhab/atticEnvironment/status/fan","Fan Off\0");
  client.publish("openhab/atticEnvironment/amps", envmessage_buff);
  }

  } // End pubFanStatus()
