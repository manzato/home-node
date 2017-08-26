#include "ProfileHandler.h"

ProfileHandler::ProfileHandler(MQTTClient* client, JsonObject& config) {
  this->client = client; 
  this->type = config.get<unsigned short>("type");
  
  JsonArray& inTopics = config["in_topics"];

  this->inTopicsCount = inTopics.size();
  this->inTopics = (char**) malloc ( this->inTopicsCount * sizeof( char* ));

  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    this->inTopics[i] = strdup(inTopics[i].asString());
  }

  this->outTopic = strdup( config["out_topic"].asString() );
  this->actuate = config.get<short>("actuate");
  this->listen = config.get<short>("listen");
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
  Serial.print("Init profile type ");
  Serial.println(this->type);

  if (this->type == YAHA_TYPE_SWITCH) {
    if (this->actuate != -1) {
      Serial.print("Actuate on ");
      Serial.println(this->actuate);
      pinMode( this->actuate, OUTPUT);
    }

    if (this->listen != -1) {
      Serial.print("Listen on ");
      Serial.println(this->listen);
      pinMode( this->listen, INPUT_PULLUP);

      //To prevent an initial "event" by the debounce logic
      this->lastValue = digitalRead(this->listen);
      this->lastDebounceValue = this->lastValue;
    }
  }

  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    Serial.print("Subscribe to ");
    Serial.println(this->inTopics[i]);
    this->client->subscribe(this->inTopics[i]);
  }
}

void ProfileHandler::loop() {
  if (this->on) {
    digitalWrite(this->actuate, HIGH);
  } else {
    digitalWrite(this->actuate, LOW);
  }

  uint8_t value = digitalRead(this->listen);

  // Maybe there is a glitch in the matrix, maybe there is an event
  if (value != this->lastValue) {
    if (value != this->lastDebounceValue) {
      this->lastDebounceCheck = millis();
    }

    if ( (millis() - this->lastDebounceCheck) > 20 ) {
      this->switchChanged();
      this->lastValue = value;
    }
    this->lastDebounceValue = value;
  }
}


void ProfileHandler::switchChanged() {
  Serial.println("switchChanged");
  this->on = ! this->on;
  this->client->publish(this->outTopic, "switch");
}

bool ProfileHandler::handle(char topic[], char payload[], int length) {
  for(int i = 0 ; i < this->inTopicsCount ; i++ ) {
    if (strcmp(this->inTopics[i], topic) == 0) { //I'm listening this topic, so I need to get to work

      if (this->type == YAHA_TYPE_SWITCH) {
        if ( payload[0] == '1') {
          this->on = true;
          Serial.println("Swicthing on..");
        } else {
          this->on = false;
          Serial.println("Swicthing off..");
        }
      }
      return true;
    }
  }
  return false;
}
