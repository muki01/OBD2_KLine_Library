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
const uint8_t init_OBD = 0x81;              // Init ISO14230
const uint8_t read_LiveData = 0x01;         // Read Live Data
const uint8_t read_FreezeFrame = 0x02;      // Read Freeze Frame Data
const uint8_t read_storedDTCs = 0x03;       // Read Stored Troubleshoot Codes
const uint8_t clear_DTCs = 0x04;            // Clear Troubleshoot Codes
const uint8_t test_OxygenSensors = 0x05;    // Test Oxygen Sensors
const uint8_t component_Monitoring = 0x06;  // Component Monitoring
const uint8_t read_pendingDTCs = 0x07;      // Read Pending Troubleshoot Codes
const uint8_t read_VehicleInfo = 0x09;      // Read Vehicle Info
const uint8_t SUPPORTED_PIDS_1_20 = 0x00;
const uint8_t SUPPORTED_PIDS_21_40 = 0x20;
const uint8_t SUPPORTED_PIDS_41_60 = 0x40;

const uint8_t supported_VehicleInfo = 0x00;  // Read Supported Vehicle Info
const uint8_t read_VIN_Count = 0x01;         // Read VIN Count
const uint8_t read_VIN = 0x02;               // Read VIN
const uint8_t read_ID_Length = 0x03;         // Read Calibration ID Length
const uint8_t read_ID = 0x04;                // Read Calibration ID
const uint8_t read_ID_Num_Length = 0x05;     // Read Calibration ID Number Length
const uint8_t read_ID_Num = 0x06;            // Read Calibration ID Number

class OBD2_KLine {
 public:
  //VehicleData car;
  OBD2_KLine(SerialType &serialStream, uint32_t baudRate, uint8_t rxPin, uint8_t txPin);

  void setDebug(Stream &serial);
  void setSerial(bool enabled);
  bool initOBD2();
  bool trySlowInit();
  bool tryFastInit();
  void writeData(uint8_t mode, uint8_t pid);
  void writeRawData(const uint8_t *dataArray, uint8_t length);
  uint8_t readData();
  void send5baud(uint8_t data);

  float getPID(uint8_t mode, uint8_t pid);
  float getLiveData(uint8_t pid);
  float getFreezeFrame(uint8_t pid);

  uint8_t readDTCs(uint8_t mode);
  uint8_t readStoredDTCs();
  uint8_t readPendingDTCs();
  String getStoredDTC(uint8_t index);
  String getPendingDTC(uint8_t index);

  bool clearDTC();

  String getVehicleInfo(uint8_t pid);

  uint8_t readSupportedLiveData();
  uint8_t readSupportedFreezeFrame();
  uint8_t readSupportedComponentMonitoring();
  uint8_t readSupportedVehicleInfo();
  uint8_t readSupportedData(uint8_t mode);
  uint8_t getSupportedData(uint8_t mode, uint8_t index);

  void setWriteDelay(uint16_t delay);
  void setDataRequestInterval(uint16_t interval);
  void setProtocol(const String &protocolName);

 private:
  SerialType *_serial;
  uint32_t _baudRate;
  uint8_t _rxPin;
  uint8_t _txPin;
  Stream *_debugSerial = nullptr;  // Debug serial port

  uint8_t resultBuffer[64] = {0};
  uint8_t errors = 0;
  bool connectionStatus = false;

  String selectedProtocol = "Automatic";
  String connectedProtocol = "";
  uint16_t _writeDelay = 5;
  uint16_t _dataRequestInterval = 60;
  String storedDTCBuffer[32];
  String pendingDTCBuffer[32];

  uint8_t supportedLiveData[32];
  uint8_t supportedFreezeFrame[32];
  uint8_t supportedComponentMonitoring[32];
  uint8_t supportedVehicleInfo[32];

  String decodeDTC(uint8_t input_byte1, uint8_t input_byte2);
  uint8_t calculateChecksum(const uint8_t *dataArray, uint8_t length);
  bool isInArray(const uint8_t *dataArray, uint8_t length, uint8_t value);
  String convertBytesToHexString(const uint8_t *dataArray, uint8_t length);
  String convertHexToAscii(const uint8_t *dataArray, uint8_t length);
  void clearEcho();
  void debugPrint(const char *msg);
  void debugPrint(const __FlashStringHelper *msg);
  void debugPrintln(const char *msg);
  void debugPrintln(const __FlashStringHelper *msg);
  void debugPrintHex(uint8_t val);    // Hexadecimal output
  void debugPrintHexln(uint8_t val);  // Hexadecimal + newline
};

#endif  // OBD2_KLINE_H