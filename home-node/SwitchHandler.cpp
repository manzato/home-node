#include "SwitchHandler.h"

#define DEBOUNCE_TIME_MS 20

void SwitchHandler::setOn() {
  this->on = true;
  Serial.print("Switching ");
  Serial.print(this->actuate);
  Serial.println(" on");
  digitalWrite(this->actuate, this->invert ? LOW : HIGH);
  if (strlen(this->outTopic) > 0) {
    this->client->publish(this->outTopic, "on");
  }
}

void SwitchHandler::setOff() {
  this->on = false;
  Serial.print("Switching ");
  Serial.print(this->actuate);
  Serial.println(" off");
  digitalWrite(this->actuate, this->invert ? HIGH : LOW);
  if (strlen(this->outTopic) > 0) {
    this->client->publish(this->outTopic, "off");
  }
}

void SwitchHandler::toggle() {
  if ( this->on ) {
    this->setOff();
  } else {
    this->setOn();
  }
}

void SwitchHandler::doHandle(char topic[], char payload[], int length) {
  if ( payload[0] == '1') {
    this->setOn();
  } else {
    this->setOff();
  }
}

void SwitchHandler::setup(JsonObject config) {
  super::setup(config);

  this->actuate = config.getMember(F("actuate")).as<short>();
  this->listen = config.getMember(F("listen")).as<short>();
  this->on= config.getMember(F("on")).as<bool>();
  this->invert = config.getMember(F("invert")).as<bool>();

  Serial.print(F("Switch"));
  if (this->listen != -1) {
    Serial.print(F(" listening on pin "));
    Serial.print(this->listen);
  }
  if (this->actuate != -1) {
    Serial.print(F(" which actuates on pin "));
    Serial.print(this->actuate);
  }
  Serial.print(F(" and initial state "));
  Serial.print(this->on ? F("ON") : F("OFF"));

  if (this->invert) {
    Serial.print(F(" (inverted)"));
  }
  Serial.println();
}

void SwitchHandler::init() {
  super::init();

  if (this->actuate != -1) {
    Serial.print(F("Actuate on "));
    Serial.println(this->actuate);
    //Set initial state
    if (this->on) {
      this->setOn();
    } else {
      this->setOff();
    }
    pinMode( this->actuate, OUTPUT);
  }

  if (this->listen != -1) {
    Serial.print(F("Listen on "));
    Serial.println(this->listen);
    pinMode( this->listen, INPUT);

    //To prevent an initial "event" by the debounce logic
    this->lastValue = digitalRead(this->listen);
    this->lastDebounceValue = this->lastValue;
  }
}

void SwitchHandler::loop() {
  // Check for a switch change
  uint8_t value = digitalRead(this->listen);

  // Maybe there is a glitch in the matrix, maybe there is an event
  if (value != this->lastValue) {
    if (value != this->lastDebounceValue) {
      this->lastDebounceCheck = millis();
    }

    if ( (millis() - this->lastDebounceCheck) > DEBOUNCE_TIME_MS ) {
      this->toggle();
      this->lastValue = value;
    }
    this->lastDebounceValue = value;
  }
}
