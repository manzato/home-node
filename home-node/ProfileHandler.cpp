#include "ProfileHandler.h"

#include "SwitchHandler.h"
  
ProfileHandler::~ProfileHandler() {
  Serial.println("Freeing resources");

  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    this->client->unsubscribe(this->inTopics[i]);
    free(this->inTopics[i]);
  }
  free(this->inTopics);
};

void ProfileHandler::setMqttClient(MQTTClient* client) {
  this->client = client;
};

void ProfileHandler::setup(JsonObject& config) {
  JsonArray& inTopics = config["in_topics"];

  this->inTopicsCount = inTopics.size();
  this->inTopics = (char**) malloc (this->inTopicsCount * sizeof( char* ));

  for(int i = 0 ; i < this->inTopicsCount ; i++) {
    this->inTopics[i] = strdup(inTopics[i].asString());
  }

  this->outTopic = strdup( config["out_topic"].asString());
};

void ProfileHandler::init() { 
  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    Serial.print("Subscribe to ");
    Serial.println(this->inTopics[i]);
    this->client->subscribe(this->inTopics[i]);
  }
};

/* Factory method for ProfileHandler's */
ProfileHandler*  ProfileHandler::createHandler(short int type) {
  switch(type) {
    case YAHA_TYPE_SWITCH:
      return new SwitchHandler();
  }
  Serial.print("Unknown handler type: ");
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

