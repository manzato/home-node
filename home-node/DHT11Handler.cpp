#include "DHT11Handler.h"

#define DEBOUNCE_TIME_MS 20

void DHT11Handler::doHandle(char topic[], char payload[], int length) {

}

void DHT11Handler::setup(JsonObject& config) {
  super::setup(config);
  this->listen = config.get<short>("listen");

  Serial.print("Setting up a DHT11");
  if (this->listen != -1) {
    Serial.print("listening on pin ");
    Serial.print(this->listen);
  }
}

void DHT11Handler::init() {
  super::init();

  if (this->listen == -1) {
    return;
  }
  Serial.print("Listen on ");
  Serial.println(this->listen);
  //pinMode( this->listen, INPUT);
  this->dht = new DHT_Unified(this->listen, DHT11);
  this->dht->begin();
  sensor_t sensor;
  this->dht->temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.print  (F("Max Freq:    ")); Serial.print(sensor.min_delay); Serial.println(F("ms"));
  Serial.println(F("------------------------------------"));
  this->minDelay = sensor.min_delay / 1000;
  this->lastMeassurementMs = millis();
}

void DHT11Handler::loop() {
  if (this->listen == -1) {
    return;
  }
  unsigned long now = millis();
  //Prevent locking if millis() overflows
  if (now < this->lastMeassurementMs ) {
    this->lastMeassurementMs = now;
  }

  if ( now - this->lastMeassurementMs < this->minDelay ) {
    return;
  }

  sensors_event_t event;
  this->dht->temperature().getEvent(&event);
  this->lastMeassurementMs = millis();
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    if (strlen(this->outTopic) > 0) {
      char buff[10];
      this->client->publish(this->outTopic, dtostrf(event.temperature, 4, 2, buff));
    }
  } 
}
