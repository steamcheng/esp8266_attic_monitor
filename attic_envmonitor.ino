/* Attic monitor: 18 Jul 2016 

  This controls an ESP8266 in the garage which monitors 
  temperature, humidity.
  
  DO NOT FORGET TO INITIALIZE NON-STANDARD SDA/SCL PIN SETUP IN Wire.begin(sda, scl).

*/

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
// #include <SFE_BMP180.h>
// #include <Wire.h>
#include <DHT.h>

/************************* DHT22 config *********************************/

#define DHTTYPE DHT22   // DHT Shield uses DHT 22 sensor
#define DHTPIN D4       // DHT Shield uses pin D4

// Setup I2C bus (for BMP? )
// const int sclPin = D6;
// const int sdaPin = D7;


//Declare variables:
unsigned long lasttime;
char message_buff[100];
char envmessage_buff[100];
int ctr = 0;

// Create a DHT class
DHT dht(DHTPIN, DHTTYPE);

// Create an SFE_BMP180 object, here called "pressure":
// SFE_BMP180 pressure;

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

  // Initialize the sensor (it is important to get calibration values stored on the device).
  
/*  Wire.begin(sdaPin, sclPin);  // Since not using the standard sda/scl pins, must initialize them here.
  
  if (pressure.begin()){
    Serial.println("BMP180 init success");
}
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  } */

}

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
  // Insert a call to the getTempPress() function
// getTempPress();

  }

 client.loop();
}


 void pubTempHum() {
 
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
 
  
 if (isnan(f) || isnan(h)) {
    Serial.println("Failed to read from DHT");
	client.publish("openhab/atticEnvironment/status","Attic is NaN!\0");
//    ctr = ctr+1;
}
else{
	client.publish("openhab/atticEnvironment/status","Attic is alive!\0");
}

 
// Temp and humidity stuff here
   String fpubString = String(f);
   fpubString.toCharArray(envmessage_buff, fpubString.length()+1);
   Serial.print(F("\nSending DHT temperature ")); Serial.print(f);
   client.publish("openhab/atticEnvironment/temp", envmessage_buff);
   Serial.print(F("\nSending DHT humidity "));  Serial.println(h);
  String hpubString = String(h); 
  hpubString.toCharArray(envmessage_buff, hpubString.length()+1);  
   client.publish("openhab/atticEnvironment/humidity", envmessage_buff);
 
  } 

/*  
// Create this function to read and publish from the BMP180
void getTempPress(){
  
  char bmpmessage_buff[100];
  char status;
  double T,Tf,P,Pa;

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
  //    Serial.print("Temperature: ");
  //    Serial.print(T,2);
  //    Serial.print(" deg C, ");
      Tf = ((9.0/5.0)*T+32.0);
  //   Serial.print(Tf,2);
  //   Serial.println(" deg F");
      // Publish as "BMPtemp"
      
   // Start a pressure measurement:
   // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
   // If request is successful, the number of ms to wait is returned.
   // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

   // Retrieve the completed pressure measurement:
   // Note that the measurement is stored in the variable P.
   // Note also that the function requires the previous temperature measurement (T).
   // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
   // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
     // Print out the measurement:
     //     Serial.print("Absolute pressure: ");
     //   Serial.print(P,2);
     //   Serial.print(" mb, ");
          Pa = (P*0.0295333727);
     //     Serial.print(Pa,2);
     //     Serial.print(" inHg");
          // publish as "Altimeter"

        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
  
//  delay(30000);  // Pause for 5 seconds.  I think this delay is already built into the other sketch.


   // temp and pressure stuff here
   String tfpubString = String(Tf);
   tfpubString.toCharArray(bmpmessage_buff, tfpubString.length()+1);
   Serial.print(F("\nSending BMP temperature ")); Serial.print(Tf); Serial.print(" Deg F");
   client.publish("openhab/atticEnvironment/bmptemp", bmpmessage_buff);
   Serial.print(F("\nSending BMP pressure "));  Serial.print(Pa); Serial.println(" inHg");
  String papubString = String(Pa); 
  papubString.toCharArray(bmpmessage_buff, papubString.length()+1);  
   client.publish("openhab/atticEnvironment/bmppress", bmpmessage_buff); 

}
*/
