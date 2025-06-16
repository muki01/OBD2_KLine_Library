#ifndef OBD2_KLINE_H
#define OBD2_KLINE_H

#include <Arduino.h>
// ==== OBD2 Mods ====
const byte init_OBD = 0x81;          // Init ISO14230
const byte read_LiveData = 0x01;     // Read Live Data
const byte read_FreezeFrame = 0x02;  // Read Freeze Frame Data
const byte read_DTCs = 0x03;         // Read Troubleshoot Codes
const byte clear_DTCs = 0x04;        // Clear Troubleshoot Codes
const byte read_VehicleInfo = 0x09;  // Read Vehicle Info

class OBD2_KLine {
 public:
  OBD2_KLine(HardwareSerial &serialPort, long baudRate, uint8_t rxPin, uint8_t txPin);

  void beginSerial();
  bool init_OBD2();
  void writeData(const byte mode, const byte pid);
  int readData();
  void send5baud(uint8_t data);
  int getPID(byte pid);

  void setWriteDelay(uint16_t delay);
  void setDataRequestInterval(uint16_t interval);
  void setProtocol(const String &protocolName);

 private:
  HardwareSerial &_serial;
  long _baudRate;
  bool _customPins;
  uint8_t _rxPin;
  uint8_t _txPin;

  byte resultBuffer[64] = {0};
  int errors = 0;
  bool connectionStatus = false;

  String protocol = "ISO9141";
  uint16_t _writeDelay = 5;
  uint16_t _dataRequestInterval = 60;
  byte calculateChecksum(const byte data[], int length);
  void clearEcho();
};

#endif  // OBD2_KLINE_H