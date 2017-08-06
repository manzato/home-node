
#define MQTT_MAX_TRANSFER_SIZE 80
#define MQTT_MAX_PACKET_SIZE 1024

#define MAX_HANDLERS 8
#define CONFIG_TOPIC_PREFIX "/config/"

#include <system.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "ProfileHandler.h"

const char* ssid = "guifi";
const char* password = "guinet123";
const char* mqtt_server = "192.168.43.249";

long lastMsg = 0;
char msg[50];
int value = 0;
unsigned int handlersCount = 0;
ProfileHandler* handlers[MAX_HANDLERS];
char clientId[13];
char* configTopic;

WiFiClient wifiClient;
PubSubClient client(wifiClient);


//The clientId generation was taken from https://github.com/marvinroger/homie-esp8266
void clientIdSetup() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(clientId, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  configTopic = (char*) malloc(strlen(clientId) + strlen(CONFIG_TOPIC_PREFIX) + 1);
  sprintf(configTopic, "%s%s", CONFIG_TOPIC_PREFIX, clientId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(BUILTIN_LED, OUTPUT); 

  //Generates the clientId and the configTopic
  clientIdSetup();
  
  //Connect to wifi
  WiFi.begin(ssid, password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(handleMqttMessages);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId)) {
      Serial.println("connected");
      
      //Subscribe to the configuration topic
      Serial.print("Subscribing to config topic: >");
      Serial.print(configTopic);
      Serial.println("<");
      client.subscribe(configTopic);

      delay(100);
      //Announce ourselves, hopefully someone will reply and will start doing fun stuff
      Serial.print("Broadcasting..");
      client.publish("/broadcast", clientId);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  // Checks for network and reconnects if it isn't present
  reconnect();

  // Gives the mqtt client cycles to process
  client.loop();

  // Gives the handlers cycles to process their on-going tasks
  for(int i = 0 ; i < handlersCount  ; i++) {
    handlers[i]->loop();
  }
}

void handleMqttConfigMessages(char* topic, byte* payload, unsigned int length) {
  Serial.print("New configuration received!: ");
  Serial.println( (char*) payload);
  DynamicJsonBuffer jsonBuffer;
  JsonArray& profiles = jsonBuffer.parseArray((char*)payload);
  if (! profiles.success() ) {
    Serial.print("Failed to parse JSON: >");
    Serial.print((char*)payload);
    Serial.println("<");
    return;
  }

  // Cleanup / remove any existing handlers, which includes disconnecting from MQTT
  for(int i = 0 ; i < handlersCount  ; i++) {
    delete handlers[i];
  }
  handlersCount = profiles.size();

  //Instantiate the required handlers
  for(int i = 0 ; i < handlersCount ; i++ ) {
    Serial.print("Setting up profile handler #");
    Serial.println(i);
    JsonObject& config = profiles[i];

    handlers[i] = new ProfileHandler(
      &client, 
      config      
    );
  }

  //initialize handlers
  for(int i = 0 ; i < handlersCount  ; i++) {
    handlers[i]->init();
  }
}

// Handles all MQTT messages (including config messages)
void handleMqttMessages(char* topic, byte* payload, unsigned int length) {
  //Let the handlers process the request if it is for them
  for(int i = 0 ; i < handlersCount  ; i++) {
    //Handler returns true if the request was for him
    if (handlers[i]->handle(topic, payload, length)) {
      return;
    }
  }

  if ( strcmp(configTopic, topic) == 0) { 
    return handleMqttConfigMessages(topic, payload, length);   
  }

  Serial.print("Unhandled message on '");
  Serial.print(topic);
  Serial.print("' >");
  //If nothing handled this so far, lets print it to the Serial and call it done
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("<");

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}
