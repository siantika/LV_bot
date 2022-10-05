#ifndef COMINTERFACE_H
#define COMINTERFACE_H
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "header.h"

#define BAUDRATE_SIM800 9600 // bps

#define MAX_SMS_CHAR 15
#define PIN_UART_SIM_RX 4
#define PIN_UART_SIM_TX 5 

class ComInterface
{
private:
  String mPhone_number;
  String mMsg;
  String mContent_of_msg;
  String mData_in;
  String mParse_data;
  void serialFlush();

public:
  ComInterface();
  String readSMS();
  String getPhone();
  void sendSMS(String &messages, String &phoneNum);
  void init(void);
  void sleepSIM800(byte sleep_mode);
  void replySerial(void);
  void hangUpcall(void);
  void phoneCall(String &phoneNum);
};

#endif