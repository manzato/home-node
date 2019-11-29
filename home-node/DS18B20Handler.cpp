#include "DS18B20Handler.h"

void DS18B20Handler::doHandle(char topic[], char payload[], int length) {

}

void DS18B20Handler::setup(JsonObject config) {
  super::setup(config);
  this->listen = config.getMember("listen").as<short>();

  Serial.print(F("DS18B20"));
  if (this->listen != -1) {
    Serial.print(F(" listening on pin "));
    Serial.print(this->listen);
  }
  Serial.println();
}

void DS18B20Handler::init() {
  super::init();

  if (this->listen == -1) {
    return;
  }
  Serial.print(F("Listen on "));
  Serial.println(this->listen);
  //pinMode( this->listen, INPUT);

  this->oneWire = new OneWire(this->listen);
  this->sensor = new DallasTemperature(this->oneWire);
  this->sensor->setResolution(12);
  
  this->minDelay = 1000 * 2;
  this->lastMeassurementMs = millis();

  //while(! this->searchSensor()) {
    delay(1000);
  //}
}

boolean DS18B20Handler::searchSensor() {
  byte i;
  boolean present = false;
  byte data[12];
  byte addr[8];

  Serial.print("Looking for 1-Wire devices...\n\r");
  while(this->oneWire->search(addr)) {
    Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
    present = true;
    for( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC is not valid!\n");
        return false;
    }
  }
  Serial.print("\n\r\n\rThat's it.\r\n");
  this->oneWire->reset_search();
  return present;
}


void DS18B20Handler::loop() {
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

  this->lastMeassurementMs = millis();

  this->sensor->requestTemperatures();
  
  float temp = this->sensor->getTempCByIndex(0);

  

  char buff[5];
  this->client->publish(this->outTopic, dtostrf(temp, 4, 2, buff));
  Serial.print(F("Temperature: "));
  Serial.println(temp);

/*
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
  */
}
