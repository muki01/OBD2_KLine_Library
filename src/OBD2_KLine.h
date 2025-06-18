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

  void setDebug(Stream &serial);
  void beginSerial();
  bool initOBD2();
  bool trySlowInit();
  bool tryFastInit();
  void resetSerialLine();
  void writeData(const byte mode, const byte pid);
  int readData();
  void send5baud(uint8_t data);
  int getPID(byte pid);
  int readDTCs();
  String getDTC(int index);

  void setWriteDelay(uint16_t delay);
  void setDataRequestInterval(uint16_t interval);
  void setProtocol(const String &protocolName);

 private:
  HardwareSerial &_serial;
  long _baudRate;
  bool _customPins;
  uint8_t _rxPin;
  uint8_t _txPin;
  Stream *_debugSerial = nullptr;  // Debug serial port

  byte resultBuffer[64] = {0};
  int errors = 0;
  bool connectionStatus = false;

  String protocol = "ISO9141";
  uint16_t _writeDelay = 5;
  uint16_t _dataRequestInterval = 60;
  String dtcBuffer[32];

  String decodeDTC(byte input_byte1, byte input_byte2);
  byte calculateChecksum(const byte data[], int length);
  void clearEcho();
  void debugPrint(const char *msg);
  void debugPrintln(const char *msg);
  void debugPrintHex(byte val);    // Hexadecimal output
  void debugPrintHexln(byte val);  // Hexadecimal + newline
};

#endif  // OBD2_KLINE_H