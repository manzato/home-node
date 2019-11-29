/*
 * A DS18B20Handler is the interface to a DS18B20 temp sensor
 *
 * Author "Guillermo Manzato <manzato@gmail.com>"
*/

#ifndef DS18B20Handler_h
#define DS18B20Handler_h

#include <DallasTemperature.h>
#include <OneWire.h>

#include "ProfileHandler.h"

class DS18B20Handler: public ProfileHandler {
  private:
    typedef ProfileHandler super;
    short int listen;
    unsigned long lastMeassurementMs;
    unsigned long minDelay;
    OneWire* oneWire;
    DallasTemperature* sensor;
    boolean searchSensor();
    
  public:
    void setup(JsonObject config);
    void init();
    void loop();
    void doHandle(char topic[], char payload[], int length);
};

#endif
