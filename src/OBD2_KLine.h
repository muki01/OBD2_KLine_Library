#ifndef OBD2_KLINE_H
#define OBD2_KLINE_H

#include <Arduino.h>

//#include "VehicleData.h"

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
#include <AltSoftSerial.h>
#define SerialType AltSoftSerial
#else
#define SerialType HardwareSerial
#endif

// ==== OBD2 Mods ====
const byte init_OBD = 0x81;              // Init ISO14230
const byte read_LiveData = 0x01;         // Read Live Data
const byte read_FreezeFrame = 0x02;      // Read Freeze Frame Data
const byte read_storedDTCs = 0x03;       // Read Stored Troubleshoot Codes
const byte clear_DTCs = 0x04;            // Clear Troubleshoot Codes
const byte test_OxygenSensors = 0x05;    // Test Oxygen Sensors
const byte component_Monitoring = 0x06;  // Component Monitoring
const byte read_pendingDTCs = 0x07;      // Read Pending Troubleshoot Codes
const byte read_VehicleInfo = 0x09;      // Read Vehicle Info
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
  //VehicleData car;
  OBD2_KLine(SerialType &serialStream, long baudRate, uint8_t rxPin, uint8_t txPin);

  void setDebug(Stream &serial);
  void setSerial(bool enabled);
  bool initOBD2();
  bool trySlowInit();
  bool tryFastInit();
  void writeData(byte mode, byte pid);
  void writeRawData(const byte *dataArray, int length);
  int readData();
  void send5baud(uint8_t data);

  float getPID(byte mode, byte pid);
  float getLiveData(byte pid);
  float getFreezeFrame(byte pid);

  int readDTCs(byte mode);
  int readStoredDTCs();
  int readPendingDTCs();
  String getStoredDTC(int index);
  String getPendingDTC(int index);

  bool clearDTC();

  String getVehicleInfo(byte pid);

  int readSupportedLiveData();
  int readSupportedFreezeFrame();
  int readSupportedComponentMonitoring();
  int readSupportedVehicleInfo();
  int readSupportedData(byte mode);
  byte getSupportedData(byte mode, int index);

  void setWriteDelay(uint16_t delay);
  void setDataRequestInterval(uint16_t interval);
  void setProtocol(const String &protocolName);

 private:
  SerialType *_serial;
  long _baudRate;
  uint8_t _rxPin;
  uint8_t _txPin;
  Stream *_debugSerial = nullptr;  // Debug serial port

  byte resultBuffer[64] = {0};
  int errors = 0;
  bool connectionStatus = false;

  String selectedProtocol = "Automatic";
  String connectedProtocol = "";
  uint16_t _writeDelay = 5;
  uint16_t _dataRequestInterval = 60;
  String storedDTCBuffer[32];
  String pendingDTCBuffer[32];

  byte supportedLiveData[32];
  byte supportedFreezeFrame[32];
  byte supportedComponentMonitoring[32];
  byte supportedVehicleInfo[32];

  String decodeDTC(byte input_byte1, byte input_byte2);
  byte calculateChecksum(const byte *dataArray, int length);
  bool isInArray(const byte *dataArray, int length, byte value);
  String convertBytesToHexString(const byte *dataArray, int length);
  String convertHexToAscii(const byte *dataArray, int length);
  void clearEcho();
  void debugPrint(const char *msg);
  void debugPrint(const __FlashStringHelper *msg);
  void debugPrintln(const char *msg);
  void debugPrintln(const __FlashStringHelper *msg);
  void debugPrintHex(byte val);    // Hexadecimal output
  void debugPrintHexln(byte val);  // Hexadecimal + newline
};

#endif  // OBD2_KLINE_H