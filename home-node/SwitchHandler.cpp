#include "SwitchHandler.h"

#define DEBOUNCE_TIME_MS 20

void SwitchHandler::setOn() {
  this->on = true;
  Serial.print("Switching ");
  Serial.print(this->actuate);
  Serial.println(" on");
  this->client->publish(this->outTopic, "on");
};

void SwitchHandler::setOff() {
  this->on = false;
  Serial.print("Switching ");
  Serial.print(this->actuate);
  Serial.println(" off");
  this->client->publish(this->outTopic, "off");
};

void SwitchHandler::toggle() {
  if ( this->on ) {
    this->setOff();
  } else {
    this->setOn();
  }
};
    
void SwitchHandler::doHandle(char topic[], char payload[], int length) {
  if ( payload[0] == '1') {
    this->setOn();
  } else {
    this->setOff();
  }  
};

void SwitchHandler::setup(JsonObject& config) {
  super::setup(config);

  this->actuate = config.get<short>("actuate");
  this->listen = config.get<short>("listen");
  this->on = config.get<boolean>("on");

  Serial.print("Setting up a switch listening on pin ");
  Serial.print(this->listen);
  Serial.print(" which actuates pin ");
  Serial.print(this->actuate);
  Serial.print(" and initial state ");
  Serial.println(this->on ? "ON" : "OFF");
};

void SwitchHandler::init() {
  super::init();
  
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

void SwitchHandler::loop() {
  //Update the output PIN with the current state
  if (this->on) {
    digitalWrite(this->actuate, HIGH);
  } else {
    digitalWrite(this->actuate, LOW);
  }

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
};
