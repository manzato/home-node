#include "ProfileHandler.h"

#include "SwitchHandler.h"
#include "DHT11Handler.h"
#include "DS18B20Handler.h"

ProfileHandler::~ProfileHandler() {
  Serial.println(F("Freeing resources"));

  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    this->client->unsubscribe(this->inTopics[i]);
    free(this->inTopics[i]);
  }
  free(this->inTopics);
};

void ProfileHandler::setMqttClient(MQTTClient* client) {
  this->client = client;
};

void ProfileHandler::setup(JsonObject config) {
  JsonArray inTopics = config.getMember("in_topics").as<JsonArray>();

  this->inTopicsCount = inTopics.size();
  Serial.print(F("have "));
  Serial.println( this->inTopicsCount);

  this->inTopics = (char**) malloc (this->inTopicsCount * sizeof( char* ));

  for(int i = 0 ; i < this->inTopicsCount ; i++) {
    this->inTopics[i] = strdup(inTopics[i].as<const char*>());
  }

  this->outTopic = strdup( config[F("out_topic")].as<const char*>());
};

void ProfileHandler::init() { 
  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    Serial.print(F("Subscribe to "));
    Serial.println(this->inTopics[i]);
    this->client->subscribe(this->inTopics[i]);
  }
};

/* Factory method for ProfileHandler's */
ProfileHandler*  ProfileHandler::createHandler(short int type) {
  switch(type) {
    case YAHA_TYPE_SWITCH:
      return new SwitchHandler();
    case YAHA_TYPE_DHT11_SENSOR:
      return new DHT11Handler();
    case YAHA_TYPE_DS18B20_SENSOR:
      return new DS18B20Handler();
  }
  Serial.print(F("Unknown handler type: "));
  Serial.println(type);
  return NULL;
};

bool ProfileHandler::handle(char topic[], char payload[], int length) {
  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    if (strcmp(this->inTopics[i], topic) == 0) { //I'm listening this topic, so I need to get to work
      this->doHandle(topic, payload, length);
      return true;
    }
  }
  return false;
};
