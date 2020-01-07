
#define MAX_HANDLERS 10
#define CONFIG_TOPIC_PREFIX "config/"


#include <MQTTClient.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "ProfileHandler.h"

#include "DHT11Handler.h"
#include "Customize.h" //Defines MQTT_IP, WIFI_SSID and WIFI_PASSWORD

unsigned int handlersCount = 0;
ProfileHandler* handlers[MAX_HANDLERS];
char deviceId[10];
char* configTopic;
unsigned long lastBroadcast = 0;

WiFiClient wifiClient;
MQTTClient client(1024); //max package size

void clientIdSetup() {
  sprintf(deviceId, "yaha-%04d", DEVICE_ID);
  configTopic = (char*) malloc(strlen(deviceId) + strlen(CONFIG_TOPIC_PREFIX) + 1);
  sprintf(configTopic, "%s%s", CONFIG_TOPIC_PREFIX, deviceId);
};

void setup() {
  Serial.begin(115200);
  Serial.println();

  //Generates the clientId and the configTopic
  clientIdSetup();

  //Set wifi to client mode only
  WiFi.mode(WIFI_STA);
  ArduinoOTA.setHostname(deviceId);

  //Connect to wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  client.begin(MQTT_SERVER, MQTT_PORT, wifiClient);
  client.onMessageAdvanced(handleMqttMessages);

  Serial.print(F("Connecting"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.print(F("! Connected with IP address: "));
  Serial.println(WiFi.localIP());

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = F("sketch");
    } else { // U_SPIFFS
      type = F("filesystem");
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println(F("Auth Failed"));
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println(F("Begin Failed"));
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println(F("Connect Failed"));
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println(F("Receive Failed"));
    } else if (error == OTA_END_ERROR) {
      Serial.println(F("End Failed"));
    }
  });

  ArduinoOTA.begin();
};

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    if (client.connect(deviceId, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println(F("connected"));

      //Subscribe to the configuration topic
      Serial.print(F("Subscribing to config topic: >"));
      Serial.print(configTopic);
      Serial.println(F("<"));
      client.subscribe(configTopic);

      delay(50);

      checkForConfig(true);
    } else {
      Serial.print(F("failed, rc="));
      Serial.print(client.connected());
      Serial.println(F(" try again in 20 seconds"));
      // Wait 20 seconds before retrying
      delay(20 * 1000);
    }
  }
};



void addPhotocell(bool force) {

}

void checkForConfig(bool force) {
  if ( ! force && handlersCount != 0 ) {
    return;
  }

  const unsigned long now = millis();

  //Prevent locking if millis() overflows
  if (now < lastBroadcast ) {
    lastBroadcast = now;
  }

  if ( force || lastBroadcast == 0 || (now - lastBroadcast > 15 * 1000) ) {
    //Announce ourselves, hopefully someone will reply and will start doing fun stuff
    lastBroadcast = now;
    if (force) {
      Serial.println(F("Forced broadcasting.."));
    } else {
      Serial.println(F("Broadcasting.."));
    }

    client.publish(F("broadcast"), deviceId);
  }
}

void loop() {
  // Checks for network and reconnects if it isn't present
  if (! client.connected()) {
    reconnect();
  }

  delay(10); //Apparently this helps with wifi stability...

  //Check for OTA updates
  ArduinoOTA.handle();

  // Gives the mqtt client cycles to process
  client.loop();

  checkForConfig(false);

  // Gives the handlers cycles to process their on-going tasks
  for(int i = 0 ; i < handlersCount  ; i++) {
    handlers[i]->loop();
  }
};

void handleMqttConfigMessages(char topic[], char payload[], int length) {
  Serial.print(F("New configuration received!: "));
  Serial.println( payload);
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (DeserializationError::Ok != error) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(error.c_str());
      Serial.print(F("Failed to parse JSON: >"));
      Serial.print(payload);
      Serial.println(F("<"));
      return;
  }

  JsonArray profiles = doc.as<JsonArray>();

  // Cleanup / remove any existing handlers, which  includes disconnecting from MQTT
  for(int i = 0 ; i < handlersCount  ; i++) {
    delete handlers[i];
  }
  handlersCount = profiles.size();

  //Instantiate the required handlers
  for(int i = 0 ; i < handlersCount ; i++ ) {
    Serial.print(F("Setting up profile handler #"));
    Serial.print(i);
    Serial.print(F(": "));
    JsonObject config = profiles[i];

    //Serial.println(config["type"].as<short int>());
    handlers[i] = ProfileHandler::createHandler( config.getMember(F("type")));
    handlers[i]->setMqttClient(&client);
    handlers[i]->setup(config);
    Serial.println();
  }

  //initialize handlers
  for(int i = 0 ; i < handlersCount  ; i++) {
    Serial.print(F("Initializing profile handler #"));
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

  Serial.print(F("Unhandled message on '"));
  Serial.print(topic);
  Serial.print("' >");
  //If nothing handled this so far, lets print it to the Serial and call it done
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("<");
};
