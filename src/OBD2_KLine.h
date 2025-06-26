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
const byte SUPPORTED_PIDS_1_20 = 0x00;
const byte SUPPORTED_PIDS_21_40 = 0x20;
const byte SUPPORTED_PIDS_41_60 = 0x40;

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

  float getLiveData(byte pid);
  float getFreezeFrame(byte pid);
  float getPID(byte mode, byte pid);

  int readDTCs();
  String getDTC(int index);
  bool clearDTC();

  int getSupportedLiveData();
  int getSupportedFreezeFrame();
  int getSupportedVehicleInfo();
  int getSupportedData(byte mode);

  String convertBytesToHexString(byte *dataArray, int length);
  String convertHexToAscii(byte *dataArray, int length);
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

  String protocol = "Automatic";
  uint16_t _writeDelay = 5;
  uint16_t _dataRequestInterval = 60;
  String dtcBuffer[32];
  byte supportedLiveData[32];
  byte supportedFreezeFrame[32];
  byte supportedVehicleInfo[32];

  String decodeDTC(byte input_byte1, byte input_byte2);
  byte calculateChecksum(const byte data[], int length);
  bool isInArray(byte arr[], int size, byte value);
  void clearEcho();
  void debugPrint(const char *msg);
  void debugPrintln(const char *msg);
  void debugPrintHex(byte val);    // Hexadecimal output
  void debugPrintHexln(byte val);  // Hexadecimal + newline
};

#endif  // OBD2_KLINE_H