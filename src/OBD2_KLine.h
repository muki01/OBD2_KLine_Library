#ifndef OBD2_KLINE_H
#define OBD2_KLINE_H

#include <Arduino.h>

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
#include <AltSoftSerial.h>
#define SerialType AltSoftSerial
#else
#define SerialType HardwareSerial
#endif

// ==== OBD2 Mods ====
const uint8_t read_LiveData = 0x01;              // Show current live data
const uint8_t read_FreezeFrame = 0x02;           // Show freeze frame data
const uint8_t read_storedDTCs = 0x03;            // Show stored Diagnostic Trouble Codes (DTCs)
const uint8_t clear_DTCs = 0x04;                 // Clear Diagnostic Trouble Codes and stored values
const uint8_t test_OxygenSensors = 0x05;         // Test results, oxygen sensor monitoring (non-CAN only)
const uint8_t test_OtherComponents = 0x06;       // Test results, other component/system monitoring (for CAN)
const uint8_t read_pendingDTCs = 0x07;           // Show pending Diagnostic Trouble Codes
const uint8_t control_OnBoardComponents = 0x08;  // Control operation of on-board component/system
const uint8_t read_VehicleInfo = 0x09;           // Request vehicle information
const uint8_t read_PermanentDTCs = 0x0A;         // Show permanent Diagnostic Trouble Codes

const uint8_t SUPPORTED_PIDS_1_20 = 0x00;
const uint8_t SUPPORTED_PIDS_21_40 = 0x20;
const uint8_t SUPPORTED_PIDS_41_60 = 0x40;
const uint8_t SUPPORTED_PIDS_61_80 = 0x60;
const uint8_t SUPPORTED_PIDS_81_100 = 0x80;

const uint8_t read_VIN_Count = 0x01;      // Read VIN Count
const uint8_t read_VIN = 0x02;            // Read VIN
const uint8_t read_ID_Length = 0x03;      // Read Calibration ID Length
const uint8_t read_ID = 0x04;             // Read Calibration ID
const uint8_t read_ID_Num_Length = 0x05;  // Read Calibration ID Number Length
const uint8_t read_ID_Num = 0x06;         // Read Calibration ID Number

const uint8_t defaultInitAddress = 0x33;

class OBD2_KLine {
 public:
  OBD2_KLine(SerialType &serialStream, uint32_t baudRate, uint8_t rxPin, uint8_t txPin);

  void setDebug(Stream &serial);
  void setSerial(bool enabled);
  bool initOBD2(uint8_t moduleAddress = defaultInitAddress);
  bool trySlowInit(uint8_t moduleAddress = defaultInitAddress);
  bool tryFastInit(uint8_t moduleAddress = defaultInitAddress);
  void writeData(uint8_t mode, uint8_t pid);
  void writeRawData(const uint8_t *dataArray, uint8_t length, uint8_t checksumType);
  uint8_t readData();
  bool compareData(const uint8_t *dataArray, uint8_t length);
  void send5baud(uint8_t data);
  int read5baud();

  float getPID(uint8_t mode, uint8_t pid);
  float getLiveData(uint8_t pid);
  float getFreezeFrame(uint8_t pid);

  uint8_t readDTCs(uint8_t mode);
  uint8_t readStoredDTCs();
  uint8_t readPendingDTCs();
  String getStoredDTC(uint8_t index);
  String getPendingDTC(uint8_t index);

  bool clearDTCs();

  String getVehicleInfo(uint8_t pid);

  uint8_t readSupportedLiveData();
  uint8_t readSupportedFreezeFrame();
  uint8_t readSupportedOxygenSensors();
  uint8_t readSupportedOtherComponents();
  uint8_t readSupportedOnBoardComponents();
  uint8_t readSupportedVehicleInfo();
  uint8_t readSupportedData(uint8_t mode);
  uint8_t getSupportedData(uint8_t mode, uint8_t index);

  void setByteWriteInterval(uint16_t interval);
  void setInterByteTimeout(uint16_t interval);
  void setReadTimeout(uint16_t timeoutMs);
  void setProtocol(const String &protocolName);
  void updateConnectionStatus(bool messageReceived);

  uint8_t initMsg[4] = {0xC1, defaultInitAddress, 0xF1, 0x81};  // ISO14230-Fast init message

 private:
  SerialType *_serial;
  uint32_t _baudRate;
  uint8_t _rxPin;
  uint8_t _txPin;
  Stream *_debugSerial = nullptr;  // Debug serial port

  uint8_t resultBuffer[160] = {0};
  uint8_t unreceivedDataCount = 0;
  bool connectionStatus = false;

  String selectedProtocol = "Automatic";
  String connectedProtocol = "";
  uint16_t _byteWriteInterval = 5;
  uint16_t _interByteTimeout = 60;
  uint16_t _readTimeout = 1000;
  String storedDTCBuffer[32];
  String pendingDTCBuffer[32];

  uint8_t supportedLiveData[32];
  uint8_t supportedFreezeFrame[32];
  uint8_t supportedOxygenSensor[32];
  uint8_t supportedOtherComponents[32];
  uint8_t supportedControlComponents[32];
  uint8_t supportedVehicleInfo[32];

  uint8_t checksum8_XOR(const uint8_t *dataArray, int length);
  uint8_t checksum8_Modulo256(const uint8_t *dataArray, int length);
  uint8_t checksum8_TwosComplement(const uint8_t *dataArray, int length);

  String decodeDTC(uint8_t input_byte1, uint8_t input_byte2);
  bool isInArray(const uint8_t *dataArray, uint8_t length, uint8_t value);
  String convertBytesToHexString(const uint8_t *dataArray, uint8_t length);
  String convertHexToAscii(const uint8_t *dataArray, uint8_t length);
  void clearEcho(int length);
  void debugPrint(const char *msg);
  void debugPrint(const __FlashStringHelper *msg);
  void debugPrintln(const char *msg);
  void debugPrintln(const __FlashStringHelper *msg);
  void debugPrintHex(uint8_t val);    // Hexadecimal output
  void debugPrintHexln(uint8_t val);  // Hexadecimal + newline
};

#endif  // OBD2_KLINE_H