#include "OBD2_KLine.h"

OBD2_KLine::OBD2_KLine(SerialType &serialPort, uint32_t baudRate, uint8_t rxPin, uint8_t txPin)
    : _serial(&serialPort), _rxPin(rxPin), _txPin(txPin), _baudRate(baudRate) {
  // Start serial
  setSerial(true);
}

void OBD2_KLine::setSerial(bool enabled) {
  if (enabled) {
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
    _serial->begin(_baudRate);
#else
    _serial->begin(_baudRate, SERIAL_8N1, _rxPin, _txPin);
#endif

  } else {
    _serial->end();
    pinMode(_rxPin, INPUT_PULLUP);
    pinMode(_txPin, OUTPUT);
    digitalWrite(_txPin, HIGH);
    delay(3000);
  }
}

bool OBD2_KLine::initOBD2() {
  if (connectionStatus) return true;

  debugPrintln(F("Initializing OBD2..."));

  if (selectedProtocol == "Automatic" || selectedProtocol == "ISO14230_Slow" || selectedProtocol == "ISO9141") {
    if (trySlowInit()) return true;
  }

  if (selectedProtocol == "Automatic" || selectedProtocol == "ISO14230_Fast") {
    if (tryFastInit()) return true;
  }

  debugPrintln(F("❌ No Protocol Matched. Initialization Failed."));
  debugPrintln(F(""));
  return false;
}

bool OBD2_KLine::trySlowInit() {
  debugPrintln(F("🔁 Trying ISO9141 / ISO14230_Slow"));

  setSerial(false);
  send5baud(0x33);
  setSerial(true);

  if (!readData()) return false;

  if (resultBuffer[0] != 0x55) return false;

  String detectedProtocol = (resultBuffer[1] == resultBuffer[2]) ? "ISO9141" : "ISO14230_Slow";
  debugPrint(F("✅ Protocol Detected: "));
  debugPrintln(detectedProtocol.c_str());

  _serial->write(~resultBuffer[2]);

  if (!readData()) return false;

  if (resultBuffer[0]) {
    connectionStatus = true;
    connectedProtocol = detectedProtocol;
    debugPrintln(F("✅ Connection established with car"));
    return true;
  }

  debugPrintln(F("❌ No response after KW2 write"));
  return false;
}

bool OBD2_KLine::tryFastInit() {
  debugPrintln(F("🔁 Trying ISO14230_Fast"));

  setSerial(false);

  digitalWrite(_txPin, LOW);
  delay(25);
  digitalWrite(_txPin, HIGH);
  delay(25);

  setSerial(true);
  writeData(init_OBD, 0x00);

  if (!readData()) return false;

  if (resultBuffer[3] == 0xC1) {
    debugPrintln(F("✅ Protocol Detected: ISO14230_Fast"));
    debugPrintln(F("✅ Connection established with car"));
    connectionStatus = true;
    connectedProtocol = "ISO14230_Fast";
    return true;
  }

  return false;
}

void OBD2_KLine::writeRawData(const uint8_t *dataArray, uint8_t length) {
  uint8_t sendData[length + 1];
  memcpy(sendData, dataArray, length);
  sendData[length] = calculateChecksum(dataArray, length);

  debugPrint(F("Sending Raw Data: "));
  for (size_t i = 0; i < length + 1; i++) {
    _serial->write(sendData[i]);
    debugPrintHex(sendData[i]);
    debugPrint(" ");
    delay(_writeDelay);
  }
  debugPrintln(F(""));

  clearEcho();
}

void OBD2_KLine::writeData(uint8_t mode, uint8_t pid) {
  uint8_t message[7] = {0};
  size_t length = (mode == read_FreezeFrame || mode == test_OxygenSensors) ? 7 : (mode == init_OBD || mode == read_storedDTCs || mode == clear_DTCs || mode == read_pendingDTCs) ? 5 : 6;

  if (selectedProtocol == "ISO9141") {
    message[0] = (mode == read_FreezeFrame) ? 0x69 : 0x68;
    message[1] = 0x6A;
  } else if (selectedProtocol == "ISO14230_Fast" || selectedProtocol == "ISO14230_Slow") {
    message[0] = (mode == read_FreezeFrame) ? 0xC3 : (mode == init_OBD || mode == read_storedDTCs || mode == clear_DTCs || mode == read_pendingDTCs) ? 0xC1 : 0xC2;
    message[1] = 0x33;
  }

  message[2] = 0xF1;
  message[3] = mode;
  if (length > 5) message[4] = pid;
  if (length == 7) message[5] = 0x00;

  message[length - 1] = calculateChecksum(message, length - 1);

  debugPrint(F("Sending Data: "));
  for (size_t i = 0; i < length; i++) {
    _serial->write(message[i]);
    debugPrintHex(message[i]);
    debugPrint(F(" "));
    delay(_writeDelay);
  }
  debugPrintln(F(""));

  clearEcho();
}

uint8_t OBD2_KLine::readData() {
  debugPrintln(F("Reading..."));
  unsigned long startMillis = millis();
  int bytesRead = 0;

  // Wait for data for 1 second
  while (millis() - startMillis < 1000) {
    if (_serial->available() > 0) {
      unsigned long lastByteTime = millis();
      memset(resultBuffer, 0, sizeof(resultBuffer));
      updateConnectionStatus(true);

      // Read all data
      debugPrint(F("✅ Received Data: "));
      while (millis() - lastByteTime < _dataRequestInterval) {  // Wait for new data for 60ms
        if (_serial->available() > 0) {                         // If new data is available
          if (bytesRead >= sizeof(resultBuffer)) {              // Stop if buffer is full
            debugPrintln(F("⚠️ Buffer is full. Stopping data reception."));
            return bytesRead;
          }

          resultBuffer[bytesRead] = _serial->read();
          debugPrintHex(resultBuffer[bytesRead]);
          debugPrint(F(" "));
          bytesRead++;
          lastByteTime = millis();  // Reset last byte_time
        }
      }

      debugPrintln(F("✅ Data reception completed."));
      return bytesRead;
    }
  }

  // If no data is received within 1 second
  debugPrintln(F("❌ OBD2 Timeout!"));
  updateConnectionStatus(false);
  return 0;
}

void OBD2_KLine::clearEcho() {
  int result = _serial->available();
  if (result > 0) {
    for (int i = 0; i < result; i++) {
      uint8_t readedByte = _serial->read();
    }
    debugPrintln(F("Echo Data Cleared"));
  } else {
    debugPrintln(F("Not Received Echo Data"));
  }
}

float OBD2_KLine::getLiveData(uint8_t pid) {
  return getPID(read_LiveData, pid);
}

float OBD2_KLine::getFreezeFrame(uint8_t pid) {
  return getPID(read_FreezeFrame, pid);
}

float OBD2_KLine::getPID(uint8_t mode, uint8_t pid) {
  writeData(mode, pid);

  int len = readData();
  if (len <= 0) {
    return -1;  // Data not received
  }

  if (resultBuffer[4] != pid) {
    return -2;  // Unexpected PID
  }

  uint8_t A = 0, B = 0, C = 0, D = 0;

  if (mode == read_LiveData) {
    int dataBytesLen = len - 6;
    A = (dataBytesLen >= 1) ? resultBuffer[5] : 0;
    B = (dataBytesLen >= 2) ? resultBuffer[6] : 0;
    C = (dataBytesLen >= 3) ? resultBuffer[7] : 0;
    D = (dataBytesLen >= 4) ? resultBuffer[8] : 0;
  } else if (mode == read_FreezeFrame) {
    int dataBytesLen = len - 7;
    A = (dataBytesLen >= 1) ? resultBuffer[6] : 0;
    B = (dataBytesLen >= 2) ? resultBuffer[7] : 0;
    C = (dataBytesLen >= 3) ? resultBuffer[8] : 0;
    D = (dataBytesLen >= 4) ? resultBuffer[9] : 0;
  }

  switch (pid) {
    case 0x01:                                        // Monitor Status Since DTC Cleared (bit encoded)
    case 0x02:                                        // Monitor Status Since DTC Cleared (bit encoded)
    case 0x03:                                        // Fuel System Status (bit encoded)
      return A;                                       //
    case 0x04:                                        // Engine Load (%)
      return A * 100 / 255;                           //
    case 0x05:                                        // Coolant Temperature (°C)
      return A - 40;                                  //
    case 0x06:                                        // Short Term Fuel Trim Bank 1 (%)
    case 0x07:                                        // Long Term Fuel Trim Bank 1 (%)
    case 0x08:                                        // Short Term Fuel Trim Bank 2 (%)
    case 0x09:                                        // Long Term Fuel Trim Bank 2 (%)
      return (int)((int8_t)A * 100 / 128);            //
    case 0x0A:                                        // Fuel Pressure (kPa)
      return A * 3;                                   //
    case 0x0B:                                        // Intake Manifold Absolute Pressure (kPa)
      return A;                                       //
    case 0x0C:                                        // RPM
      return ((A * 256) + B) / 4;                     //
    case 0x0D:                                        // Speed (km/h)
      return A;                                       //
    case 0x0E:                                        // Timing Advance (°)
      return (int8_t)A / 2;                           //
    case 0x0F:                                        // Intake Air Temperature (°C)
      return A - 40;                                  //
    case 0x10:                                        // MAF Flow Rate (grams/sec)
      return ((A * 256) + B) / 100;                   //
    case 0x11:                                        // Throttle Position (%)
      return A * 100 / 255;                           //
    case 0x12:                                        // Commanded Secondary Air Status (bit encoded)
    case 0x13:                                        // Oxygen Sensors Present 2 Banks (bit encoded)
      return A;                                       //
    case 0x14:                                        // Oxygen Sensor 1A Voltage (V)
    case 0x15:                                        // Oxygen Sensor 2A Voltage (V)
    case 0x16:                                        // Oxygen Sensor 3A Voltage (V)
    case 0x17:                                        // Oxygen Sensor 4A Voltage (V)
    case 0x18:                                        // Oxygen Sensor 5A Voltage (V)
    case 0x19:                                        // Oxygen Sensor 6A Voltage (V)
    case 0x1A:                                        // Oxygen Sensor 7A Voltage (V)
    case 0x1B:                                        // Oxygen Sensor 8A Voltage (V)
      return A / 200;                                 // Volt to mV (V*1000), float hesap gerekirse fonksiyon
    case 0x1C:                                        // OBD Standards This Vehicle Conforms To (bit encoded)
    case 0x1D:                                        // Oxygen Sensors Present 4 Banks (bit encoded)
    case 0x1E:                                        // Auxiliary Input Status (bit encoded)
      return A;                                       //
    case 0x1F:                                        // Run Time Since Engine Start (seconds)
    case 0x21:                                        // Distance Traveled With MIL On (km)
      return (A * 256) + B;                           //
    case 0x22:                                        // Fuel Rail Pressure (kPa)
    case 0x23:                                        // Fuel Rail Gauge Pressure (kPa)
      return ((A * 256) + B) / 10;                    //
    case 0x24:                                        // Oxygen Sensor 1B (ratio voltage)
    case 0x25:                                        // Oxygen Sensor 2B
    case 0x26:                                        // Oxygen Sensor 3B
    case 0x27:                                        // Oxygen Sensor 4B
    case 0x28:                                        // Oxygen Sensor 5B
    case 0x29:                                        // Oxygen Sensor 6B
    case 0x2A:                                        // Oxygen Sensor 7B
    case 0x2B:                                        // Oxygen Sensor 8B
      return ((A * 256) + B) * 0.0000305 * 1000;      // ratio * 1000 (mV), float önerilir
    case 0x2C:                                        // Commanded EGR (%)
      return A * 100 / 255;                           //
    case 0x2D:                                        // EGR Error (%)
      return (int8_t)A * 100 / 128;                   //
    case 0x2E:                                        // Commanded Evaporative Purge (%)
    case 0x2F:                                        // Fuel Tank Level Input (%)
      return A * 100 / 255;                           //
    case 0x30:                                        // Warm-ups Since Codes Cleared (count)
      return A;                                       //
    case 0x31:                                        // Distance Traveled Since Codes Cleared (km)
      return (A * 256) + B;                           //
    case 0x32: {                                      // Evap System Vapor Pressure (Pa)
      int16_t signedValue = (int16_t)((A << 8) | B);  //
      return signedValue / 4;                         //
    }
    case 0x33:                            // Absolute Barometric Pressure (kPa)
      return A;                           //
    case 0x34:                            // Oxygen Sensor 1C (current)
    case 0x35:                            // Oxygen Sensor 2C
    case 0x36:                            // Oxygen Sensor 3C
    case 0x37:                            // Oxygen Sensor 4C
    case 0x38:                            // Oxygen Sensor 5C
    case 0x39:                            // Oxygen Sensor 6C
    case 0x3A:                            // Oxygen Sensor 7C
    case 0x3B:                            // Oxygen Sensor 8C
      return ((A * 256) + B) * 0.000488;  //
    case 0x3C:                            // Catalyst Temperature Bank 1 Sensor 1 (°C)
    case 0x3D:                            // Catalyst Temperature Bank 2 Sensor 1 (°C)
    case 0x3E:                            // Catalyst Temperature Bank 1 Sensor 2 (°C)
    case 0x3F:                            // Catalyst Temperature Bank 2 Sensor 2 (°C)
      return (A * 256) + B - 40;          //
    default:                              //
      return -4;                          // Unknown PID
  }
}

uint8_t OBD2_KLine::readDTCs(uint8_t mode) {
  // Request: C2 33 F1 03 F3
  // example Response: 87 F1 11 43 01 70 01 34 00 00 72
  // example Response: 87 F1 11 43 00 00 72
  int dtcCount = 0;
  String *targetArray = nullptr;

  if (mode == read_storedDTCs) {
    targetArray = storedDTCBuffer;
  } else if (mode == read_pendingDTCs) {
    targetArray = pendingDTCBuffer;
  } else {
    return -1;  // Invalid mode
  }

  writeData(mode, 0x00);

  int len = readData();
  if (len >= 3) {
    for (int i = 0; i < len - 5; i += 2) {
      uint8_t b1 = resultBuffer[4 + i];
      uint8_t b2 = resultBuffer[4 + i + 1];

      if (b1 == 0 && b2 == 0) break;

      targetArray[dtcCount++] = decodeDTC(b1, b2);
    }
  }

  return dtcCount;
}

uint8_t OBD2_KLine::readStoredDTCs() {
  return readDTCs(0x03);
}

uint8_t OBD2_KLine::readPendingDTCs() {
  return readDTCs(0x07);
}

bool OBD2_KLine::clearDTC() {
  writeData(clear_DTCs, 0x00);
  int len = readData();
  if (len >= 3) {
    if (resultBuffer[3] == 0x44) {
      return true;
    }
  }
  return false;
}

uint8_t OBD2_KLine::calculateChecksum(const uint8_t *dataArray, uint8_t length) {
  uint8_t checksum = 0;
  for (int i = 0; i < length; i++) {
    checksum += dataArray[i];
  }
  return checksum % 256;
}

String OBD2_KLine::decodeDTC(uint8_t input_byte1, uint8_t input_byte2) {
  String ErrorCode = "";
  const char type_lookup[4] = {'P', 'C', 'B', 'U'};
  const char digit_lookup[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  ErrorCode += type_lookup[(input_byte1 >> 6) & 0x03];
  ErrorCode += digit_lookup[(input_byte1 >> 4) & 0x03];
  ErrorCode += digit_lookup[input_byte1 & 0x0F];
  ErrorCode += digit_lookup[(input_byte2 >> 4) & 0x0F];
  ErrorCode += digit_lookup[input_byte2 & 0x0F];

  return ErrorCode;
}

String OBD2_KLine::getStoredDTC(uint8_t index) {
  if (index >= 0) return storedDTCBuffer[index];
  return "";
}

String OBD2_KLine::getPendingDTC(uint8_t index) {
  if (index >= 0) return pendingDTCBuffer[index];
  return "";
}

void OBD2_KLine::setWriteDelay(uint16_t delay) {
  _writeDelay = delay;
}

void OBD2_KLine::setDataRequestInterval(uint16_t interval) {
  _dataRequestInterval = interval;
}

void OBD2_KLine::setProtocol(const String &protocolName) {
  selectedProtocol = protocolName;
  connectionStatus = false;  // Reset connection status
  connectedProtocol = "";    // Reset connected protocol
  debugPrintln(("Protocol set to: " + selectedProtocol).c_str());
}

void OBD2_KLine::send5baud(uint8_t data) {
  uint8_t even = 1;  // for calculating parity bit
  uint8_t bits[10];

  bits[0] = 0;  // start bit
  bits[9] = 1;  // stop bit

  // 7-bit data and parity calculation
  for (int i = 1; i <= 7; i++) {
    bits[i] = (data >> (i - 1)) & 1;
    even ^= bits[i];
  }

  bits[8] = (even == 0) ? 1 : 0;  // parity bit

  debugPrint(F("5 Baud Init for Module 0x"));
  debugPrintHex(data);
  debugPrint(F(": "));

  // Set txPin as output
  pinMode(_txPin, OUTPUT);

  for (int i = 0; i < 10; i++) {
    debugPrint(bits[i] ? "1" : "0");
    digitalWrite(_txPin, bits[i] ? HIGH : LOW);
    delay(200);
  }

  debugPrintln(F(""));
}

void OBD2_KLine::setDebug(Stream &serial) {
  _debugSerial = &serial;
}

void OBD2_KLine::debugPrint(const char *msg) {
  if (_debugSerial) _debugSerial->print(msg);
}

void OBD2_KLine::debugPrint(const __FlashStringHelper *msg) {
  if (_debugSerial) _debugSerial->print(msg);
}

void OBD2_KLine::debugPrintln(const char *msg) {
  if (_debugSerial) _debugSerial->println(msg);
}

void OBD2_KLine::debugPrintln(const __FlashStringHelper *msg) {
  if (_debugSerial) _debugSerial->println(msg);
}

void OBD2_KLine::debugPrintHex(uint8_t val) {
  if (_debugSerial) {
    if (val < 0x10) _debugSerial->print("0");
    _debugSerial->print(val, HEX);
  }
}

void OBD2_KLine::debugPrintHexln(uint8_t val) {
  if (_debugSerial) {
    debugPrintHex(val);
    _debugSerial->println();
  }
}

uint8_t OBD2_KLine::readSupportedLiveData() {
  return readSupportedData(read_LiveData);
}

uint8_t OBD2_KLine::readSupportedFreezeFrame() {
  return readSupportedData(read_FreezeFrame);
}

uint8_t OBD2_KLine::readSupportedComponentMonitoring() {
  return readSupportedData(component_Monitoring);
}

uint8_t OBD2_KLine::readSupportedVehicleInfo() {
  return readSupportedData(read_VehicleInfo);
}

uint8_t OBD2_KLine::readSupportedData(uint8_t mode) {
  int supportedCount = 0;
  int pidIndex = 0;
  int startByte = 0;
  int arraySize = 32;  // Size of supported data arrays
  uint8_t *targetArray = nullptr;

  if (mode == read_LiveData) {
    startByte = 5;
    targetArray = supportedLiveData;
  } else if (mode == read_FreezeFrame) {
    startByte = 6;
    targetArray = supportedFreezeFrame;
  } else if (mode == read_VehicleInfo) {
    startByte = 6;
    targetArray = supportedVehicleInfo;
  } else if (mode == component_Monitoring) {
    startByte = 6;
    targetArray = supportedComponentMonitoring;
  } else {
    return -1;  // Invalid mode
  }

  writeData(mode, SUPPORTED_PIDS_1_20);
  if (readData()) {
    for (int i = 0; i < 4; i++) {
      uint8_t value = resultBuffer[i + startByte];
      for (int bit = 7; bit >= 0; bit--) {
        if ((value >> bit) & 1) {
          targetArray[supportedCount++] = pidIndex + 1;
        }
        pidIndex++;
      }
    }
  }

  if (isInArray(targetArray, arraySize, 0x20)) {
    writeData(mode, SUPPORTED_PIDS_21_40);
    if (readData()) {
      for (int i = 0; i < 4; i++) {
        uint8_t value = resultBuffer[i + startByte];
        for (int bit = 7; bit >= 0; bit--) {
          if ((value >> bit) & 1) {
            targetArray[supportedCount++] = pidIndex + 1;
          }
          pidIndex++;
        }
      }
    }
  }

  if (isInArray(targetArray, arraySize, 0x40)) {
    writeData(mode, SUPPORTED_PIDS_41_60);
    if (readData()) {
      for (int i = 0; i < 4; i++) {
        uint8_t value = resultBuffer[i + startByte];
        for (int bit = 7; bit >= 0; bit--) {
          if ((value >> bit) & 1) {
            targetArray[supportedCount++] = pidIndex + 1;
          }
          pidIndex++;
        }
      }
    }
  }

  return supportedCount;
}

uint8_t OBD2_KLine::getSupportedData(uint8_t mode, uint8_t index) {
  if (mode == 0x01) {
    if (index >= 0) return supportedLiveData[index];
  } else if (mode == 0x02) {
    if (index >= 0) return supportedFreezeFrame[index];
  } else if (mode == 0x06) {
    if (index >= 0) return supportedComponentMonitoring[index];
  } else if (mode == 0x09) {
    if (index >= 0) return supportedVehicleInfo[index];
  }
  return 0;
}

bool OBD2_KLine::isInArray(const uint8_t *dataArray, uint8_t length, uint8_t value) {
  for (int i = 0; i < length; i++) {
    if (dataArray[i] == value) {
      return true;
    }
  }
  return false;
}

String OBD2_KLine::getVehicleInfo(uint8_t pid) {
  // Request: C2 33 F1 09 02 F1
  // example Response: 87 F1 11 49 02 01 00 00 00 31 06
  //                   87 F1 11 49 02 02 41 31 4A 43 D5
  //                   87 F1 11 49 02 03 35 34 34 34 A8
  //                   87 F1 11 49 02 04 52 37 32 35 C8
  //                   87 F1 11 49 02 05 32 33 36 37 E6

  uint8_t dataArray[64];
  int messageCount;
  int arrayNum = 0;

  if (pid == 0x02) {
    messageCount = 5;
  } else if (pid == 0x04 || pid == 0x06) {
    if (pid == 0x04) {
      writeData(read_VehicleInfo, read_ID_Length);
    } else if (pid == 0x06) {
      writeData(read_VehicleInfo, read_ID_Num_Length);
    } else {
      return "";
    }

    if (readData()) {
      messageCount = resultBuffer[5];
    } else {
      return "";
    }
  }

  writeData(read_VehicleInfo, pid);

  if (readData()) {
    for (int j = 0; j < messageCount; j++) {
      if (pid == 0x02 && j == 0) {
        dataArray[arrayNum++] = resultBuffer[9];
        continue;
      }
      for (int i = 1; i <= 4; i++) {
        dataArray[arrayNum++] = resultBuffer[i + 5 + j * 11];
      }
    }
  }

  if (pid == 0x02 || pid == 0x04) {
    return convertHexToAscii(dataArray, arrayNum);
  } else if (pid == 0x06) {
    return convertBytesToHexString(dataArray, arrayNum);
  }
  return "";
}

void OBD2_KLine::updateConnectionStatus(bool messageReceived) {
  if (messageReceived) {
    errors = 0;
    // if (!connectionStatus) {
    //   connectionStatus = true;
    //   debugPrintln(F("✅ Connection established."));
    // }
  } else {
    if (!connectionStatus) return;  // No need to update if not connected

    errors++;
    debugPrint(F("⚠️ Not received data, error count: "));
    debugPrintln(String(errors).c_str());
    if (errors > 2 && connectionStatus) {
      connectionStatus = false;
      errors = 0;
      debugPrintln(F("⛔ Connection lost."));
    }
  }
}

String OBD2_KLine::convertHexToAscii(const uint8_t *dataArray, uint8_t length) {
  String asciiString = "";
  for (int i = 0; i < length; i++) {
    uint8_t b = dataArray[i];
    if (b >= 0x20 && b <= 0x7E) {  // Printable ASCII range
      asciiString += (char)b;
    }
  }
  return asciiString;
}

String OBD2_KLine::convertBytesToHexString(const uint8_t *dataArray, uint8_t length) {
  String hexString = "";
  for (int i = 0; i < length; i++) {
    if (dataArray[i] < 0x10) hexString += "0";  // Pad leading zero
    hexString += String(dataArray[i], HEX);
  }
  hexString.toUpperCase();
  return hexString;
}