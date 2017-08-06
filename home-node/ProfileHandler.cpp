#include "ProfileHandler.h"

ProfileHandler::ProfileHandler(PubSubClient* client, JsonObject& config) {
  this->client = client; 
  this->type = config.get<unsigned short>("type");
  
  JsonArray& inTopics = config["in_topics"];

  this->inTopicsCount = inTopics.size();
  this->inTopics = (char**) malloc ( this->inTopicsCount * sizeof( char* ));

  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    this->inTopics[i] = strdup(inTopics[i].asString());
  }
}

ProfileHandler::~ProfileHandler() {
  Serial.println("Freeing resources");
  
  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    this->client->unsubscribe(this->inTopics[i]);
    free(this->inTopics[i]);
  }
  free(this->inTopics);
}

void ProfileHandler::init() {
  Serial.print("Init profile ");
  Serial.println(this->type);

  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    Serial.print("Subscribe to ");
    Serial.println(this->inTopics[i]);
    this->client->subscribe(this->inTopics[i]);
  }
}

void ProfileHandler::loop() {
  
}

bool ProfileHandler::handle(char* topic, byte* payload, unsigned int length) {
  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    if (strcmp(this->inTopics[i], topic) == 0) {
      // Switch on the LED if an 1 was received as first character
      if ((char)payload[0] == '1') {
        Serial.println("Swicthing on..");
        digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
        // but actually the LED is on; this is because
        // it is acive low on the ESP-01)
      } else {
        Serial.println("Swicthing off..");
        digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
      }
      return true;
    }
  }
  return false;
}
