#include <Arduino.h>
#include "header.h"
#include "io_mapping.h"

// Forward function declaration
bool bacaPintu();

void setup()
{
  Serial.begin(BAUDRATE);
  // sensor pintu
  pinMode(PIN_SENSOR_PINTU, INPUT_PULLUP); 
}

void loop()
{

}

bool bacaPintu()
{
  return digitalRead(PIN_SENSOR_PINTU);
}
