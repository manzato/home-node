
#define MAX_HANDLERS 8
#define CONFIG_TOPIC_PREFIX "config/"

#include <system.h>
#include <MQTTClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "ProfileHandler.h"
#include "SwitchHandler.h"
#include "Customize.h" //Defines MQTT_IP, WIFI_SSID and WIFI_PASSWORD

unsigned int handlersCount = 0;
ProfileHandler* handlers[MAX_HANDLERS];
char clientId[13];
char* configTopic;

WiFiClient wifiClient;
MQTTClient client(512); //max package size

//The clientId generation was taken from https://github.com/marvinroger/homie-esp8266
void clientIdSetup() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(clientId, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  configTopic = (char*) malloc(strlen(clientId) + strlen(CONFIG_TOPIC_PREFIX) + 1);
  sprintf(configTopic, "%s%s", CONFIG_TOPIC_PREFIX, clientId);
};

void setup() {
  Serial.begin(115200);
  Serial.println();

  //Generates the clientId and the configTopic
  clientIdSetup();

  //Connect to wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  client.begin(MQTT_SERVER, MQTT_PORT, wifiClient);
  client.onMessageAdvanced(handleMqttMessages);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
};

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
      client.publish("broadcast", clientId);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.connected());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
};

void loop() {
  // Checks for network and reconnects if it isn't present
  if (! client.connected()) {
    reconnect();
  }

  delay(10); //Apparently this helps with wifi stability...

  // Gives the mqtt client cycles to process
  client.loop();

  // Gives the handlers cycles to process their on-going tasks
  for(int i = 0 ; i < handlersCount  ; i++) {
    handlers[i]->loop();
  }
};

void handleMqttConfigMessages(char topic[], char payload[], int length) {
  Serial.print("New configuration received!: ");
  Serial.println( payload);
  DynamicJsonBuffer jsonBuffer;
  JsonArray& profiles = jsonBuffer.parseArray((char*)payload);
  if (! profiles.success() ) {
    Serial.print("Failed to parse JSON: >");
    Serial.print(payload);
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

    handlers[i] = ProfileHandler::createHandler( config.get<unsigned short>("type"));
    handlers[i]->setMqttClient(&client);
    handlers[i]->setup(config);
  }

  //initialize handlers
  for(int i = 0 ; i < handlersCount  ; i++) {
    Serial.print("Initializing profile handler #");
    Serial.println(i);
    handlers[i]->init();
  }
};

void handleMqttMessages(MQTTClient* client, char topic[], char payload[], int length) {
  //Let the handlers process the request if it is for them
  boolean handled = false;
  for(int i = 0 ; i < handlersCount  ; i++) {
    //Handler returns true if the request was for him
    if (handlers[i]->handle(topic, payload, length)) {
      // Don't stop here, since another handler may be listening to the same topic
      handled = true;
    }
  }

  if (handled) {
    return;
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
};

