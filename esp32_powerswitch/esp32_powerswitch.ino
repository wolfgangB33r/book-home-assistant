#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define MQTT_VERSION MQTT_VERSION_3_1_1

// Wifi: SSID and password
const char* WIFI_SSID = "<YOUR_OWN_WLAN_SSID>";
const char* WIFI_PASSWORD = "<YOUR_OWN_WLAN_PWD>";

// MQTT: ID, server IP, port, username and password
const PROGMEM char* MQTT_CLIENT_ID = "powerswitch01";
const PROGMEM char* MQTT_SERVER_IP = "<YOUR_HOME_ASSISTANT_IP_ADDRESS>";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "homeassistant";
const PROGMEM char* MQTT_PASSWORD = "mqttpwd";

const char* MQTT_ACTION_SUB = "/home/powerswitch/actions/#";
const char* MQTT_ACTION_SUB_SWITCH01 = "/home/powerswitch/actions/powerswitch01";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void switch01(String action) {
  if(action.equals("ON")) {
    Serial.println(action);
    digitalWrite(D1, LOW);
  }
  else {
    Serial.println(action);
    digitalWrite(D1, HIGH);   
  }
}

// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  Serial.println(p_topic);
  p_payload[p_length] = '\0';
  String a = String((char*)p_payload);
  String topic = String(p_topic);
  if(topic.equals(MQTT_ACTION_SUB_SWITCH01)) {
    switch01(a);
  }
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
  // subscribe to topic
  if (client.subscribe(MQTT_ACTION_SUB)){
    Serial.println("Successfully subscribed to MQTT topic");
  }
}

void setup() {
  // init the serial
  Serial.begin(115200);

  pinMode(D1,  OUTPUT);
  digitalWrite(D1, HIGH);

  // init the WiFi connection
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
}

void loop() {
  
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
  
  delay(100);
  client.loop();
}
