// used to parse and send JSON message format
#include <ArduinoJson.h>
// used for accessing the temperature sensor values
#include <DHT.h>
#include <DHT_U.h>
// use the ESP32 built in WLAN module
#include <ESP8266WiFi.h>
// MQTT client implementation to send the measurements
#include <PubSubClient.h>


#define MQTT_VERSION MQTT_VERSION_3_1_1
// Wifi: SSID and password
const char* WIFI_SSID = "<YOUR_OWN_WLAN_SSID>";
const char* WIFI_PASSWORD = "<YOUR_OWN_WLAN_PWD>";

// MQTT: ID, server IP, port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "sensor1_dht22";
const PROGMEM char* MQTT_SERVER_IP = "<YOUR_HOME_ASSISTANT_IP_ADDRESS>";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "homeassistant";
const PROGMEM char* MQTT_PASSWORD = "mqttpwd";

// MQTT: topic
const PROGMEM char* MQTT_SENSOR_TOPIC = "/home/outdoor/sensor1";

// DHT - D1/GPIO5
#define DHTPIN 5
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// function called to publish the temperature and the humidity
void publishData(float p_temperature, float p_humidity) {
  // create a JSON object
  // doc : https://github.com/bblanchon/ArduinoJson/wiki/API%20Reference
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  // INFO: the data must be converted into a string; a problem occurs when using floats...
  root["temperature"] = (String)p_temperature;
  root["humidity"] = (String)p_humidity;
  root.prettyPrintTo(Serial);
  Serial.println("");
  /*
     {
        "temperature": "23.20" ,
        "humidity": "43.70"
     }
  */
  char data[200];
  root.printTo(data, root.measureLength() + 1);
  client.publish(MQTT_SENSOR_TOPIC, data, true);
  yield();
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("INFO: connected");
    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // init the serial
  Serial.begin(115200);

  dht.begin();

  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.println("INFO: IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);


  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void loop() {
  dht.begin();
  
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  // if MQTT lost connection then reconnect
  if (!client.connected()) {
    reconnect();
  }
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("ERROR: Failed to read from DHT sensor!");
  } 
  else {
    publishData(t, h);
  }
  delay(6000);
}
