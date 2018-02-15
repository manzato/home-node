/*
 * A SwitchHandler is the virtual equivalent of a "light switch"
 *
 * Author "Guillermo Manzato <manzato@gmail.com>"
*/

#ifndef SwitchHandler_h
#define SwitchHandler_h

#include <ArduinoJson.h>
#include "ProfileHandler.h"


class SwitchHandler: public ProfileHandler {
  private:
    typedef ProfileHandler super;
    boolean on;
    short int actuate;
    short int listen;
    unsigned long lastDebounceCheck;
    uint8_t lastValue;
    uint8_t lastDebounceValue;
    void setOn();
    void setOff();
    void toggle();

  public:
    void setup(JsonObject& config);
    void init();
    void loop();
    void doHandle(char topic[], char payload[], int length);
};

#endif
