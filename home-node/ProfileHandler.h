
/*
 * Author "Guillermo Manzato <manzato@gmail.com>"
 * 
 * Wrapper around PubSubClient to allow using callbacks on subscribes 
*/

#ifndef ProfileHandler_h
#define ProfileHandler_h

#include <PubSubClient.h>
#include <ArduinoJson.h>

class ProfileHandler {
  private:
    PubSubClient* client;
    unsigned short int type;
    unsigned short int inTopicsCount;
    char** inTopics;
    
  public:
    ProfileHandler(PubSubClient* client, JsonObject& config);
    ~ProfileHandler();
    void init();
    void loop();
    bool handle(char* topic, byte* payload, unsigned int length);

};

#endif
