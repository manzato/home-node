/*
 * A DHT11Handler is the interface to a DHT11 temp and humidity sensor
 *
 * Author "Guillermo Manzato <manzato@gmail.com>"
*/

#ifndef DHT11Handler_h
#define DHT11Handler_h

#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include "ProfileHandler.h"

class DHT11Handler: public ProfileHandler {
  private:
    typedef ProfileHandler super;
    short int listen;
    unsigned long lastMeassurementMs;
    unsigned long minDelay;
    DHT_Unified* dht;
    
  public:
    void setup(JsonObject& config);
    void init();
    void loop();
    void doHandle(char topic[], char payload[], int length);
};

#endif
