#include <Arduino.h>
#include "header.h"
#include "io_mapping.h"
#include "PZEM004Tv30.h"
#include "ComInterface.h"
#include "LiquidCrystal_I2C.h"

// Global variabel
// nomor handphone
bool statusKeadaanPintu;
bool statusOperasionalSMS;
uint8_t state = 0;
uint8_t stateTampilan = 0;
String NO_HP = "6285333389189"; // No HP ISI DI SINI ! (+62 ...)
String msg = "Warning! \n KD 0181 \n Tegangan hilang, Fasa R padam \n tolong di cek!";
String msgMenyala = "KD 0181 Fasa R, sudah menyala";

// buat konversi dari float ke string (pada funsgi lcdDisplay)
char str_dataFasa_R[3];
char str_dataFasa_S[3];
char str_dataFasa_T[3];

/* -------------- Inisialisasi -------------- */

/* *************** LCD 16 x 2 ************** */
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long prevTime = 0;
/* *************** akhir dari LCD 16 x 2  *************** */

/* *************** PZEM  *************** */
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

/* *************** akhir dari PZEM *************** */

/* *************** SIM800C *************** */
ComInterface SIM800C;
/* *************** akhir dari SIM800C *************** */

/* *************** Forward function declaration ************ */
bool bacaPintu();
String nungguSMS();
void bacaDataListrik(PZEM004Tv30 _pzem, dataListrik *_dataListrik);
void kirimSMS(String _kontenSMS, String _noHP);
void clearNotif();
void lcdDisplay();
/* *************** akhir dari forward function declaration ************ */

/* -------------- Akhir dari Inisialisasi  -------------- */

void setup()
{
  Serial.begin(BAUDRATE);
  // LCD 16 x 2
  lcd.begin();
  millis(); // mulai timer untuk display (LCD diperbaharui setiap 2 detik sekali)
  lcd.backlight();
  lcd.print("   Alat Ready!");

  // sensor pintu
  pinMode(PIN_SENSOR_PINTU, INPUT_PULLUP);

  // SIM800C
  delay(3000); // untuk inisiasi SIM800C
  SIM800C.init();
}

void loop()
{
  /* ************** METODE STATE MACHINE ************** */
  switch (state)
  {
  case 1:
  {
    bool statusPintu = bacaPintu();
  }
    break;
  
  default:
    break;
  }

  /* ************** AKHIR DARI METODE STATE MACHINE ************** */
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

void kirimSMS(String _kontenSMS, String _noHP)
{
  SIM800C.sendSMS(_kontenSMS, _noHP);
}

void clearNotif()
{
}

void lcdDisplay()
{
  char _dataParameter[MAX_CHAR] = "";
  String _judul = "";

  stateTampilan = stateTampilan % 3;

  if (millis() - prevTime >= INTERVAL_DISPLAY)
  {
    prevTime = millis();
    if (stateTampilan == 0)
    {
      dtostrf(dataListrik_fasa_R.tegangan, 3, 0, str_dataFasa_R);
      dtostrf(dataListrik_fasa_S.tegangan, 3, 0, str_dataFasa_S);
      dtostrf(dataListrik_fasa_T.tegangan, 3, 0, str_dataFasa_T);
      _judul = "TEGANGAN (RST)";
    }
    else if (stateTampilan == 1)
    {
      dtostrf(dataListrik_fasa_R.arus, 3, 0, str_dataFasa_R);
      dtostrf(dataListrik_fasa_S.arus, 3, 0, str_dataFasa_S);
      dtostrf(dataListrik_fasa_T.arus, 3, 0, str_dataFasa_T);
      _judul = "  ARUS (RST)";
    }
    else if (stateTampilan == 2)
    {
      dtostrf(dataListrik_fasa_R.frekuensi, 3, 0, str_dataFasa_R);
      dtostrf(dataListrik_fasa_S.frekuensi, 3, 0, str_dataFasa_S);
      dtostrf(dataListrik_fasa_T.frekuensi, 3, 0, str_dataFasa_T);
      _judul = "FREKUENSI (RST)";
    }

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(_judul);
    lcd.setCursor(0, 1);
    sprintf(_dataParameter, " %s %s %s ", str_dataFasa_R, str_dataFasa_S, str_dataFasa_T);
    lcd.print(_dataParameter);

    stateTampilan += 1; // update state display
  }
}
