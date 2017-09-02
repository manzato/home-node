
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
    
  protected:
    char** inTopics;
    unsigned short int inTopicsCount;
    MQTTClient* client;
    char* outTopic;

  public:
    ~ProfileHandler();

    void setMqttClient(MQTTClient* client);
    
    virtual void init();

    virtual void loop() = 0;

    bool handle(char topic[], char payload[], int length);

    virtual void setup(JsonObject& config);

    /** Returns an instance of the handlers specified by <i>type</i> */
    static ProfileHandler* createHandler(short int type);

    virtual void doHandle(char topic[], char payload[], int length) = 0;
};

#endif
