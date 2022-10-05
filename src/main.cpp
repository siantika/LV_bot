#include <Arduino.h>
#include "header.h"
#include "io_mapping.h"
#include "PZEM004Tv30.h"
#include "ComInterface.h"

// Global variabel
bool statusKeadaanPintu;
bool statusOperasionalSMS;

/* -------------- Inisialisasi -------------- */

/* *************** PZEM awal *************** */
// Variabel
typedef struct
{
  float tegangan;
  float arus;
  float frekuensi;
} dataListrik;

// Deklarasi objek
SoftwareSerial pzemSWSerial_R(PIN_UART_RX_FASA_R, PIN_UART_TX_FASA_R);
SoftwareSerial pzemSWSerial_S(PIN_UART_RX_FASA_S, PIN_UART_TX_FASA_S);
SoftwareSerial pzemSWSerial_T(PIN_UART_RX_FASA_T, PIN_UART_TX_FASA_T);
PZEM004Tv30 pzem_R(pzemSWSerial_R);
PZEM004Tv30 pzem_S(pzemSWSerial_S);
PZEM004Tv30 pzem_T(pzemSWSerial_T);

dataListrik dataListrik_fasa_R;
dataListrik dataListrik_fasa_S;
dataListrik dataListrik_fasa_T;

/* *************** PZEM akhir *************** */

/* *************** SIM800C awal *************** */
ComInterface SIM800C;
/* *************** SIM800C akhir *************** */

// Forward function declaration
bool bacaPintu();
void bacaDataListrik(PZEM004Tv30 _pzem, dataListrik *_dataListrik);
String nungguSMS();
void kirimSMS(String *_kontenSMS, String *_noHP);
void clearNotif();

void setup()
{
  Serial.begin(BAUDRATE);
  // sensor pintu
  pinMode(PIN_SENSOR_PINTU, INPUT_PULLUP);
  // sensor PZEM
  // SIM800C
  SIM800C.init();
}

void loop()
{
  bacaDataListrik(pzem_R, &dataListrik_fasa_R);
  bacaDataListrik(pzem_S, &dataListrik_fasa_S);
  bacaDataListrik(pzem_T, &dataListrik_fasa_T);

  Serial.println("---------------------------");
  Serial.println("Parameter \tFasa R \tFasa S \tFasa T");
  Serial.println("---------------------------");
  Serial.print("Tegangan \t");
  Serial.print(String(dataListrik_fasa_R.tegangan));
  Serial.print("\t" + String(dataListrik_fasa_S.tegangan));
  Serial.print("\t" + String(dataListrik_fasa_T.tegangan));
  Serial.println();
  Serial.print("Arus \t\t");
  Serial.print(String(dataListrik_fasa_R.arus));
  Serial.print("\t" + String(dataListrik_fasa_S.arus));
  Serial.print("\t" + String(dataListrik_fasa_T.arus));
  Serial.println();
  Serial.print("Frekuensi \t");
  Serial.print(String(dataListrik_fasa_R.frekuensi));
  Serial.print("\t" + String(dataListrik_fasa_S.frekuensi));
  Serial.print("\t" + String(dataListrik_fasa_T.frekuensi));
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
}

/* ************** Fungsi - Fungsi ************* */
void bacaDataListrik(PZEM004Tv30 _pzem, dataListrik *_dataListrik)
{
  _dataListrik->tegangan = _pzem.voltage();
  _dataListrik->arus = _pzem.current();
  _dataListrik->frekuensi = _pzem.frequency();

  // cek error pembacaan parameter pada pzem. Jika pzem tidak dipasang pada terminal listrik maka error terjadi!
  if (isnan(_dataListrik->tegangan))
  {
    _dataListrik->tegangan = 0;
  }
  if (isnan(_dataListrik->arus))
  {
    _dataListrik->arus = 0;
  }
  if (isnan(_dataListrik->frekuensi))
  {
    _dataListrik->frekuensi = 0;
  }
}

bool bacaPintu()
{
  return digitalRead(PIN_SENSOR_PINTU);
}

String nungguSMS()
{
  String _smsMasuk = SIM800C.readSMS();
  return _smsMasuk;
}

void kirimSMS(String *_kontenSMS, String *_noHP)
{
  SIM800C.sendSMS(*_kontenSMS, *_noHP);
}

void clearNotif()
{
}