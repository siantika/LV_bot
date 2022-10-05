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
bool statusListrikPadam = 0; // tidak padam = 0 || padam = 1
uint8_t state = 0;
uint8_t stateTampilan = 0;
char fasaPadam;
String NO_HP = "6285333389189"; // No HP ISI DI SINI ! (+62 ...)
String pesanListrikMenyala = "KD 0181 Tegangan sudah kembali normal";
String pesanListrikPadam;
String pesanMasuk;
String kontenSMSInfoFrekuensi;
// buat konversi dari float ke string (pada funsgi lcdDisplay)
char str_dataFasa_R[5];
char str_dataFasa_S[5];
char str_dataFasa_T[5];

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

  // trigger state
  state = 1;
}

void loop()
{
  bacaDataListrik(pzem_R, &dataListrik_fasa_R);
  bacaDataListrik(pzem_S, &dataListrik_fasa_S);
  bacaDataListrik(pzem_T, &dataListrik_fasa_T);

  /* ************** METODE STATE MACHINE ************** */

  lcdDisplay(); // timer interrupt
  Serial.println(state);
  switch (state)
  {
  case 1:
  {
    bool statusPintu = bacaPintu();

    // controller state
    if (statusPintu == STATUS_PINTU_TERTUTUP)
    {
      state = 10;
      //  Serial.println("PINTU TERUTUP");
    }

    else if (statusPintu == STATUS_PINTU_TERBUKA)
    {
      if (statusKeadaanPintu == 0)
      {
        Serial.println("PINTU TERBUKA");
        statusKeadaanPintu = 1; // tandai sudah pernah terbuka
        state = 6;
      }
      else if (statusKeadaanPintu == 1)
      {
        state = 2;
      }
    }
  }
  break;
  case 2:
  {
    // controller state
    // listrik menyala
    if ((dataListrik_fasa_R.tegangan * dataListrik_fasa_S.tegangan * dataListrik_fasa_T.tegangan) != 0 && statusListrikPadam == 0)
    {
      state = 3;
    }
    else if (dataListrik_fasa_R.tegangan == 0)
    {
      pesanListrikPadam = "Warning! \n KD 0181 \n Tegangan hilang, Fasa R padam \n tolong di cek!";
      state = 7;
    }
    else if (dataListrik_fasa_S.tegangan == 0)
    {
      pesanListrikPadam = "Warning! \n KD 0181 \n Tegangan hilang, Fasa S padam \n tolong di cek!";
      state = 7;
    }

    else if (dataListrik_fasa_T.tegangan == 0)
    {
      pesanListrikPadam = "Warning! \n KD 0181 \n Tegangan hilang, Fasa T padam \n tolong di cek!";
      state = 7;
    }
    if ((dataListrik_fasa_R.tegangan * dataListrik_fasa_S.tegangan * dataListrik_fasa_T.tegangan) != 0 && statusListrikPadam == 1)
    {
      state = 9;
    }
  }
  break;
  case 3:
  {
    clearNotif();

    // controller state
    state = 4;
  }
  break;
  case 4:
  {
    //  normalisasi huruf
    // controller state
    if (pesanMasuk == "info")
    {
      state = 5;
    }
    else
    {
      state = 1;
    }
  }
  break;
  case 5:
  {
    // String kontenSMSInfo = " Kririm pesan listrik";
    state = 1;
  }
  break;
  case 6:
  {
    String pesanAlarmPintu = "KD 0181 Pintu lv board terbuka!";
    kirimSMS(pesanAlarmPintu, NO_HP);
    state = 1;
  }
  break;
  case 7:
  {
    statusListrikPadam = 1; // tandai listrik masih padam
    if (statusOperasionalSMS == 0)
    {
      state = 8;
      statusOperasionalSMS = 1; // tandai sudah kirim sms listrik padam
    }
    else if (statusOperasionalSMS == 1)
    {
      state = 2;
    }
  }
  break;

  case 8:
  {
    kirimSMS(pesanListrikPadam, NO_HP);
    String kontenSMSInfoTegangan = "Tegangan: " + String(dataListrik_fasa_R.tegangan) + " | " + String(dataListrik_fasa_S.tegangan) + " | " + String(dataListrik_fasa_T.tegangan);
    kirimSMS(kontenSMSInfoTegangan, NO_HP);
    String kontenSMSInfoArus = "Arus: " + String(dataListrik_fasa_R.arus) + " | " + String(dataListrik_fasa_S.arus) + " | " + String(dataListrik_fasa_T.arus);
    kirimSMS(kontenSMSInfoArus, NO_HP);

    state = 2;
  }
  break;
  case 9:
  {
    kirimSMS(pesanListrikMenyala, NO_HP);
    String kontenSMSInfoTegangan = "Tegangan: " + String(dataListrik_fasa_R.tegangan) + " | " + String(dataListrik_fasa_S.tegangan) + " | " + String(dataListrik_fasa_T.tegangan);
    kirimSMS(kontenSMSInfoTegangan, NO_HP);
    String kontenSMSInfoArus = "Arus: " + String(dataListrik_fasa_R.arus) + " | " + String(dataListrik_fasa_S.arus) + " | " + String(dataListrik_fasa_T.arus);
    kirimSMS(kontenSMSInfoArus, NO_HP);

    state = 3;
  }
  break;
  case 10:
  {
    if (statusKeadaanPintu == 1)
    {
      statusKeadaanPintu = 0; // pintu sudah ditutup
      state = 2;
    }
    else if (statusKeadaanPintu == 0)
    {
      state = 2;
    }
  }
  break;
  default:
  {
    Serial.println("ada error pada state:");
    Serial.println(state);
  }
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
  statusListrikPadam = 0;   // listrik menyala
  statusOperasionalSMS = 0; // sms untuk listrik padam sudah direset
}

void lcdDisplay()
{
  char _dataParameter[MAX_CHAR] = "";
  String _judul = "";

  stateTampilan = stateTampilan % 2;

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
    // else if (stateTampilan == 2)
    // {
    //   dtostrf(dataListrik_fasa_R.frekuensi, 3, 0, str_dataFasa_R);
    //   dtostrf(dataListrik_fasa_S.frekuensi, 3, 0, str_dataFasa_S);
    //   dtostrf(dataListrik_fasa_T.frekuensi, 3, 0, str_dataFasa_T);
    //   _judul = "FREKUENSI (RST)";
    // }

    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(_judul);
    lcd.setCursor(0, 1);
    sprintf(_dataParameter, " %s %s %s ", str_dataFasa_R, str_dataFasa_S, str_dataFasa_T);
    lcd.print(_dataParameter);
    // Serial.println(_dataParameter);

    stateTampilan += 1; // update state display
  }
}
