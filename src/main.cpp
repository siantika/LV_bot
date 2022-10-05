#include <Arduino.h>
#include "header.h"
#include "io_mapping.h"
#include "PZEM004Tv30.h"

// variabel
// PZEM
typedef struct
{
  float tegangan;
  float arus;
  float frekuensi;
} dataListrik;

// PZEM
SoftwareSerial pzemSWSerial_R(PIN_UART_RX_FASA_R, PIN_UART_TX_FASA_R);
SoftwareSerial pzemSWSerial_S(PIN_UART_RX_FASA_S, PIN_UART_TX_FASA_S);
SoftwareSerial pzemSWSerial_T(PIN_UART_RX_FASA_T, PIN_UART_TX_FASA_T);
PZEM004Tv30 pzem_R(pzemSWSerial_R);
PZEM004Tv30 pzem_S(pzemSWSerial_S);
PZEM004Tv30 pzem_T(pzemSWSerial_T);

dataListrik dataListrik_fasa_R;
dataListrik dataListrik_fasa_S;
dataListrik dataListrik_fasa_T;


// Forward function declaration
bool bacaPintu();
void bacaDataListrik(PZEM004Tv30 _pzem, dataListrik _dataListrik);





void setup()
{
  Serial.begin(BAUDRATE);
  // sensor pintu
  pinMode(PIN_SENSOR_PINTU, INPUT_PULLUP);
  // sensor PZEM
}

void loop()
{
}


/* ************** Fungsi - Fungsi ************* */
void bacaDataListrik(PZEM004Tv30 _pzem, dataListrik * _dataListrik)
{
  _dataListrik->tegangan = _pzem.voltage();
  _dataListrik->arus = _pzem.current();
  _dataListrik->frekuensi = _pzem.frequency();

  if (isnan(_dataListrik->tegangan))
  {
    _dataListrik->tegangan = 0;
  }

}


bool bacaPintu()
{
  return digitalRead(PIN_SENSOR_PINTU);
}
