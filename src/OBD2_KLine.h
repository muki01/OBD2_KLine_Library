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

const byte supported_VehicleInfo = 0x00;  // Read Supported Vehicle Info
const byte read_VIN_Count = 0x01;         // Read VIN Count
const byte read_VIN = 0x02;               // Read VIN
const byte read_ID_Length = 0x03;         // Read Calibration ID Length
const byte read_ID = 0x04;                // Read Calibration ID
const byte read_ID_Num_Length = 0x05;     // Read Calibration ID Number Length
const byte read_ID_Num = 0x06;            // Read Calibration ID Number

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
  void writeRawData(const byte *dataArray, int length);
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

  String getVehicleInfo(byte pid);

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
  String convertBytesToHexString(const byte *dataArray, int length);
  String convertHexToAscii(const byte *dataArray, int length);
  void clearEcho();
  void debugPrint(const char *msg);
  void debugPrintln(const char *msg);
  void debugPrintHex(byte val);    // Hexadecimal output
  void debugPrintHexln(byte val);  // Hexadecimal + newline
};

#endif  // OBD2_KLINE_H