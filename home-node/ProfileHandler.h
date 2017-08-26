
/*
 * Author "Guillermo Manzato <manzato@gmail.com>"
 * 
 * Wrapper around PubSubClient to allow using callbacks on subscribes 
*/

#ifndef ProfileHandler_h
#define ProfileHandler_h

#include <MQTTClient.h>
#include <ArduinoJson.h>

#define YAHA_TYPE_OFF 0
#define YAHA_TYPE_SWITCH 1

class ProfileHandler {
  private:
    MQTTClient* client;
    unsigned short int type;
    unsigned short int inTopicsCount;
    unsigned long lastDebounceCheck;
    uint8_t lastValue;  
    uint8_t lastDebounceValue;
    char** inTopics;
    char* outTopic;
    short int actuate;
    short int listen;
    boolean on;
    
  public:
    ProfileHandler(MQTTClient* client, JsonObject& config);
    ~ProfileHandler();
    void init();
    void loop();
    void switchChanged();
    bool handle(char topic[], char payload[], int length);

};

#endif
