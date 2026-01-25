#include "OBD2_KLine.h"

OBD2_KLine::OBD2_KLine(SerialType &serialPort, uint32_t baudRate, uint8_t rxPin, uint8_t txPin)
    : _serial(&serialPort), _rxPin(rxPin), _txPin(txPin), _baudRate(baudRate) {
  // Start serial
  setSerial(true);
}

// ----------------------------------- Initialization functions -----------------------------------

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
  delay(5500);
  send5baud(defaultInitAddress);
  setSerial(true);

  setInterByteTimeout(30);

  if (!readData()) {
    setInterByteTimeout(60);
    return false;
  }
  if (resultBuffer[0] != 0x55) return false;

  String detectedProtocol = (resultBuffer[1] == resultBuffer[2]) ? "ISO9141" : "ISO14230_Slow";
  debugPrint(F("✅ Protocol Detected: "));
  debugPrintln(detectedProtocol.c_str());

  debugPrintln(F("Writing inverted KW2"));
  _serial->write(~resultBuffer[2]);
  delay(_byteWriteInterval);
  clearEcho(1);

  setInterByteTimeout(60);

  if (!readData()) {
    debugPrintln(F("❌ No response after KW2 write"));
    return false;
  } else {
    connectionStatus = true;
    connectedProtocol = detectedProtocol;
    debugPrintln(F("✅ Connection established with car"));
    return true;
  }
}

  // 83 F1 11 C1 EF 8F C4
bool OBD2_KLine::tryFastInit() {
  debugPrintln(F("🔁 Trying ISO14230_Fast"));

  setSerial(false);
  delay(5500);

  digitalWrite(_txPin, LOW);
  delay(25);
  digitalWrite(_txPin, HIGH);
  delay(25);

  setSerial(true);
  writeData((uint8_t[]){0x81});

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

// ----------------------------------- Basic Read/Write functions -----------------------------------

void OBD2_KLine::writeRawData(const uint8_t *dataArray, uint8_t length, uint8_t checksumType) {
  uint8_t totalLength = length;  // default no checksum
  uint8_t checksum = 0;

  switch (checksumType) {
    case 0:
      totalLength = length;
      break;
    case 1:
      checksum = checksum8_XOR(dataArray, length);
      totalLength = length + 1;
      break;
    case 2:
      checksum = checksum8_Modulo256(dataArray, length);
      totalLength = length + 1;
      break;
    case 3:
      checksum = checksum8_TwosComplement(dataArray, length);
      totalLength = length + 1;
      break;
    default:
      totalLength = length;
      break;
  }

  uint8_t sendData[totalLength];
  memcpy(sendData, dataArray, length);
  if (checksumType != 0) {
    sendData[totalLength - 1] = checksum;
  }

  debugPrint(F("\n➡️ Sending Raw Data: "));
  for (size_t i = 0; i < totalLength; i++) {
    debugPrintHex(sendData[i]);
    debugPrint(F(" "));
  }
  debugPrintln(F(""));

  for (size_t i = 0; i < totalLength; i++) {
    _serial->write(sendData[i]);
    if (i < totalLength - 1) delay(_byteWriteInterval);
  }

  clearEcho(totalLength);
}

void OBD2_KLine::writeData(const uint8_t* data, uint8_t dataLength) {
  uint8_t headerLength = 3;
  uint8_t actualLengthByteCount = useLengthInHeader ? 0 : 1;
  uint8_t fullDataLength = headerLength + actualLengthByteCount + dataLength + 1;  // +1 for checksum
  uint8_t message[fullDataLength];

  if (connectedProtocol == "ISO9141") {
    memcpy(message, header_ISO9141, headerLength);
  } else if (connectedProtocol == "ISO14230_Fast" || connectedProtocol == "ISO14230_Slow" || connectionStatus == false) {
    memcpy(message, header_ISO14230_Fast, headerLength);

    if (useLengthInHeader) {
      message[0] += dataLength;
    } else {
      message[3] = dataLength;
    }
  }

  uint8_t dataStartOffset = headerLength + actualLengthByteCount;
  memcpy(&message[dataStartOffset], data, dataLength);

  message[fullDataLength - 1] = checksum8_Modulo256(message, fullDataLength - 1);

  debugPrint(F("\n➡️ Sending Data: "));
  for (size_t i = 0; i < fullDataLength; i++) {
    debugPrintHex(message[i]);
    debugPrint(F(" "));
  }
  debugPrintln(F(""));

  for (size_t i = 0; i < fullDataLength; i++) {
    _serial->write(message[i]);
    if (i < fullDataLength - 1) delay(_byteWriteInterval);
  }

  clearEcho(fullDataLength);
}

uint8_t OBD2_KLine::readData() {
  debugPrintln(F("Reading..."));
  unsigned long startMillis = millis();
  int bytesRead = 0;

  // Wait for data for the specified timeout
  while (millis() - startMillis < _readTimeout) {
    if (_serial->available() > 0) {
      unsigned long lastByteTime = millis();
      memset(resultBuffer, 0, sizeof(resultBuffer));
      updateConnectionStatus(true);

      // Read all data
      debugPrint(F("✅ Received Data: "));
      while (millis() - lastByteTime < _interByteTimeout) {  // Wait for new data for 60ms
        if (_serial->available() > 0) {                      // If new data is available
          if (bytesRead >= sizeof(resultBuffer)) {           // Stop if buffer is full
            debugPrintln(F("\n⚠️ Buffer is full. Stopping data reception."));
            return bytesRead;
          }

          resultBuffer[bytesRead] = _serial->read();
          debugPrintHex(resultBuffer[bytesRead]);
          debugPrint(F(" "));
          bytesRead++;
          lastByteTime = millis();  // Reset last byte_time
        }
      }

      debugPrintln(F("\n✅ Data reception completed."));
      return bytesRead;
    }
  }

  // If no data is received within 1 second
  debugPrintln(F("❌ OBD2 Timeout!"));
  updateConnectionStatus(false);
  return 0;
}

void OBD2_KLine::clearEcho(uint8_t length) {
  const unsigned long byteTimeoutMs = 100;

  // Wait for the first byte
  unsigned long startTime = millis();
  while (_serial->available() == 0) {
    if (millis() - startTime >= byteTimeoutMs) {
      debugPrintln(F("❌ Echo not received"));
      return;
    }
    delayMicroseconds(100);
  }

  // First byte received, now read the rest
  debugPrint(F("🗑️ Cleared Echo Data: "));

  uint8_t readedByte;
  for (size_t readCount = 0; readCount < length; readCount++) {
    startTime = millis();

    while (_serial->available() == 0) {
      if (millis() - startTime >= byteTimeoutMs) {
        debugPrintln(F("\n❌ Echo incomplete"));
        return;
      }
      delayMicroseconds(100);
    }

    readedByte = _serial->read();
    debugPrintHex(readedByte);
    debugPrint(F(" "));
  }

  debugPrintln(F(""));
}

bool OBD2_KLine::compareData(const uint8_t *dataArray, uint8_t length) {
  for (size_t i = 0; i < length; i++) {
    if (dataArray[i] != resultBuffer[i]) {
      return false;
    }
  }
  return true;
}

// ----------------------------------- Live Data -----------------------------------

float OBD2_KLine::getLiveData(uint8_t pid) {
  return getPID(read_LiveData, pid);
}

float OBD2_KLine::getFreezeFrame(uint8_t pid) {
  return getPID(read_FreezeFrame, pid);
}

float OBD2_KLine::getPID(uint8_t mode, uint8_t pid) {
  // example Request: C2 33 F1 01 0C F3
  // example Response: 84 F1 11 41 0C 0D 58 38
  if (mode == read_LiveData) {
    writeData((uint8_t[]){mode, pid});
  } else if (mode == read_FreezeFrame) {
    writeData((uint8_t[]){mode, pid, 0x00});
  }

  int len = readData();

  if (len <= 0) return -1;                // Data not received
  if (resultBuffer[4] != pid) return -2;  // Unexpected PID

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
    case 0x01:                                      // Monitor Status Since DTC Cleared (bit encoded)
    case 0x02:                                      // Monitor Status Since DTC Cleared (bit encoded)
    case 0x03:                                      // Fuel System Status (bit encoded)
      return A;                                     //
    case 0x04:                                      // Engine Load (%)
      return A * 100.0f / 255.0f;                   //
    case 0x05:                                      // Coolant Temperature (°C)
      return A - 40.0f;                             //
    case 0x06:                                      // Short Term Fuel Trim Bank 1 (%)
    case 0x07:                                      // Long Term Fuel Trim Bank 1 (%)
    case 0x08:                                      // Short Term Fuel Trim Bank 2 (%)
    case 0x09:                                      // Long Term Fuel Trim Bank 2 (%)
      return A * 100.0f / 128.0f - 100.0f;          //
    case 0x0A:                                      // Fuel Pressure (kPa)
      return A * 3.0f;                              //
    case 0x0B:                                      // Intake Manifold Absolute Pressure (kPa)
      return A;                                     //
    case 0x0C:                                      // RPM
      return ((A * 256.0f) + B) / 4.0f;             //
    case 0x0D:                                      // Speed (km/h)
      return A;                                     //
    case 0x0E:                                      // Timing Advance (°)
      return A / 2.0f - 64.0f;                      //
    case 0x0F:                                      // Intake Air Temperature (°C)
      return A - 40.0f;                             //
    case 0x10:                                      // MAF Flow Rate (grams/sec)
      return ((A * 256.0f) + B) / 100.0f;           //
    case 0x11:                                      // Throttle Position (%)
      return A * 100.0f / 255.0f;                   //
    case 0x12:                                      // Commanded Secondary Air Status (bit encoded)
    case 0x13:                                      // Oxygen Sensors Present 2 Banks (bit encoded)
      return A;                                     //
    case 0x14:                                      // Oxygen Sensor 1A Voltage (V, %)
    case 0x15:                                      // Oxygen Sensor 2A Voltage (V, %)
    case 0x16:                                      // Oxygen Sensor 3A Voltage (V, %)
    case 0x17:                                      // Oxygen Sensor 4A Voltage (V, %)
    case 0x18:                                      // Oxygen Sensor 5A Voltage (V, %)
    case 0x19:                                      // Oxygen Sensor 6A Voltage (V, %)
    case 0x1A:                                      // Oxygen Sensor 7A Voltage (V, %)
    case 0x1B:                                      // Oxygen Sensor 8A Voltage (V, %)
      return A / 200.0f;                            // Voltage
    case 0x1C:                                      // OBD Standards This Vehicle Conforms To (bit encoded)
    case 0x1D:                                      // Oxygen Sensors Present 4 Banks (bit encoded)
    case 0x1E:                                      // Auxiliary Input Status (bit encoded)
      return A;                                     //
    case 0x1F:                                      // Run Time Since Engine Start (seconds)
    case 0x21:                                      // Distance Traveled With MIL On (km)
      return (A * 256.0f) + B;                      //
    case 0x22:                                      // Fuel Rail Pressure (kPa)
      return ((A * 256.0f) + B) * 0.079f;           //
    case 0x23:                                      // Fuel Rail Gauge Pressure (kPa)
      return ((A * 256.0f) + B) / 10.0f;            //
    case 0x24:                                      // Oxygen Sensor 1B (ratio, voltage)
    case 0x25:                                      // Oxygen Sensor 2B (ratio, voltage)
    case 0x26:                                      // Oxygen Sensor 3B (ratio, voltage)
    case 0x27:                                      // Oxygen Sensor 4B (ratio, voltage)
    case 0x28:                                      // Oxygen Sensor 5B (ratio, voltage)
    case 0x29:                                      // Oxygen Sensor 6B (ratio, voltage)
    case 0x2A:                                      // Oxygen Sensor 7B (ratio, voltage)
    case 0x2B:                                      // Oxygen Sensor 8B (ratio, voltage)
      return ((A * 256.0f) + B) / 32768.0f;         // ratio
    case 0x2C:                                      // Commanded EGR (%)
      return A * 100.0f / 255.0f;                   //
    case 0x2D:                                      // EGR Error (%)
      return A * 100.0f / 128.0f - 100.0f;          //
    case 0x2E:                                      // Commanded Evaporative Purge (%)
    case 0x2F:                                      // Fuel Tank Level Input (%)
      return A * 100.0f / 255.0f;                   //
    case 0x30:                                      // Warm-ups Since Codes Cleared (count)
      return A;                                     //
    case 0x31:                                      // Distance Traveled Since Codes Cleared (km)
      return (A * 256.0f) + B;                      //
    case 0x32:                                      // Evap System Vapor Pressure (Pa)
      return ((A * 256.0f) + B) / 4.0f;             //
    case 0x33:                                      // Absolute Barometric Pressure (kPa)
      return A;                                     //
    case 0x34:                                      // Oxygen Sensor 1C (current)
    case 0x35:                                      // Oxygen Sensor 2C
    case 0x36:                                      // Oxygen Sensor 3C
    case 0x37:                                      // Oxygen Sensor 4C
    case 0x38:                                      // Oxygen Sensor 5C
    case 0x39:                                      // Oxygen Sensor 6C
    case 0x3A:                                      // Oxygen Sensor 7C
    case 0x3B:                                      // Oxygen Sensor 8C
      return ((A * 256.0f) + B) / 32768.0f;         // ratio
    case 0x3C:                                      // Catalyst Temperature Bank 1 Sensor 1 (°C)
    case 0x3D:                                      // Catalyst Temperature Bank 2 Sensor 1 (°C)
    case 0x3E:                                      // Catalyst Temperature Bank 1 Sensor 2 (°C)
    case 0x3F:                                      // Catalyst Temperature Bank 2 Sensor 2 (°C)
      return ((A * 256.0f) + B) / 10.0f - 40.0f;    //
    case 0x41:                                      // Monitor status this drive cycle (bit encoded)
      return A;                                     //
    case 0x42:                                      // Control module voltage (V)
      return ((A * 256.0f) + B) / 1000.0f;          //
    case 0x43:                                      // Absolute load value (%)
      return ((A * 256.0f) + B) * 100.0f / 255.0f;  //
    case 0x44:                                      // Fuel/Air commanded equivalence ratio (lambda)
      return ((A * 256.0f) + B) / 32768.0f;         // ratio
    case 0x45:                                      // Relative throttle position (%)
      return A * 100.0f / 255.0f;                   //
    case 0x46:                                      // Ambient air temp (°C)
      return A - 40.0f;                             //
    case 0x47:                                      // Absolute throttle position B (%)
    case 0x48:                                      // Absolute throttle position C (%)
    case 0x49:                                      // Accelerator pedal position D (%)
    case 0x4A:                                      // Accelerator pedal position E (%)
    case 0x4B:                                      // Accelerator pedal position F (%)
    case 0x4C:                                      // Commanded throttle actuator (%)
      return A * 100.0f / 255.0f;                   //
    case 0x4D:                                      // Time run with MIL on (min)
    case 0x4E:                                      // Time since trouble codes cleared (min)
      return (A * 256.0f) + B;                      //
    case 0x4F:                                      // Max values for sensors (ratio, V, mA, kPa)
    case 0x50:                                      // Maximum value for air flow rate from mass air flow sensor (g/s)
    case 0x51:                                      // Fuel Type (bit encoded)
      return A;                                     //
    case 0x52:                                      // Ethanol fuel (%)
      return A * 100.0f / 255.0f;                   //
    case 0x53:                                      // Absolute evap system pressure (kPa)
      return ((A * 256.0f) + B) / 200.0f;           //
    case 0x54:                                      // Evap system vapor pressure (Pa)
      return (A * 256.0f) + B;                      //
    case 0x55:                                      // Short term secondary oxygen sensor trim, A: bank 1, B: bank 3 (%)
    case 0x56:                                      // Long term primary oxygen sensor trim, A: bank 1, B: bank 3 (%)
    case 0x57:                                      // Short term secondary oxygen sensor trim, A: bank 2, B: bank 4 (%)
    case 0x58:                                      // Long term secondary oxygen sensor trim, A: bank 2, B: bank 4 (%)
      return A * 100.0f / 128.0f - 100.0f;          //
    case 0x59:                                      // Fuel rail absolute pressure (kPa)
      return ((A * 256.0f) + B) * 10.0f;            //
    case 0x5A:                                      // Relative accelerator pedal position (%)
    case 0x5B:                                      // Hybrid battery pack remaining life (%)
      return A * 100.0f / 255.0f;                   //
    case 0x5C:                                      // Engine oil temperature (°C)
      return A - 40.0f;                             //
    case 0x5D:                                      // Fuel injection timing (°)
      return ((A * 256.0f) + B) / 128.0f - 210.0f;  //
    case 0x5E:                                      // Engine fuel rate (L/h)
      return ((A * 256.0f) + B) / 20.0f;            //
    case 0x5F:                                      // Emission requirements to which vehicle is designed (bit encoded)
      return A;                                     //
    case 0x61:                                      // Driver's demand engine - percent torque (%)
    case 0x62:                                      // Actual engine - percent torque (%)
      return A - 125.0f;                            //
    case 0x63:                                      // Engine reference torque (Nm)
      return (A * 256.0f) + B;                      //
    default:                                        //
      return -4;                                    // Unknown PID
  }
}

// ----------------------------------- DTCs -----------------------------------

uint8_t OBD2_KLine::readStoredDTCs() {
  return readDTCs(0x03);
}

uint8_t OBD2_KLine::readPendingDTCs() {
  return readDTCs(0x07);
}

uint8_t OBD2_KLine::readDTCs(uint8_t mode) {
  // Request: C2 33 F1 03 F3
  // example Response: 87 F1 11 43 01 70 01 34 00 00 72
  // example Response: 87 F1 11 43 00 00 CC
  int dtcCount = 0;
  String *targetArray = nullptr;

  if (mode == read_storedDTCs) {
    targetArray = storedDTCBuffer;
  } else if (mode == read_pendingDTCs) {
    targetArray = pendingDTCBuffer;
  } else {
    return -1;  // Invalid mode
  }

  writeData((uint8_t[]){mode});

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

String OBD2_KLine::getStoredDTC(uint8_t index) {
  if (index >= 0) return storedDTCBuffer[index];
  return "";
}

String OBD2_KLine::getPendingDTC(uint8_t index) {
  if (index >= 0) return pendingDTCBuffer[index];
  return "";
}

bool OBD2_KLine::clearDTCs() {
  writeData((uint8_t[]){clear_DTCs});
  int len = readData();
  if (len >= 3) {
    if (resultBuffer[3] == 0x44) {
      memset(storedDTCBuffer, 0, sizeof(storedDTCBuffer));
      memset(pendingDTCBuffer, 0, sizeof(pendingDTCBuffer));
      return true;
    }
  }
  return false;
}

// ----------------------------------- Vehicle Information -----------------------------------

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
      writeData((uint8_t[]){read_VehicleInfo, read_ID_Length});
    } else if (pid == 0x06) {
      writeData((uint8_t[]){read_VehicleInfo, read_ID_Num_Length});
    } else {
      return "";
    }

    if (readData()) {
      messageCount = resultBuffer[5];
    } else {
      return "";
    }
  }

  writeData((uint8_t[]){read_VehicleInfo, pid});

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

// ----------------------------------- Supported PIDs -----------------------------------

uint8_t OBD2_KLine::readSupportedLiveData() {
  return readSupportedData(read_LiveData);
}

uint8_t OBD2_KLine::readSupportedFreezeFrame() {
  return readSupportedData(read_FreezeFrame);
}

uint8_t OBD2_KLine::readSupportedOxygenSensors() {
  return readSupportedData(test_OxygenSensors);
}

uint8_t OBD2_KLine::readSupportedOtherComponents() {
  return readSupportedData(test_OtherComponents);
}

uint8_t OBD2_KLine::readSupportedOnBoardComponents() {
  return readSupportedData(control_OnBoardComponents);
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

  if (mode == read_LiveData) {  // Mode 01
    startByte = 5;
    targetArray = supportedLiveData;
  } else if (mode == read_FreezeFrame) {  // Mode 02
    startByte = 6;
    targetArray = supportedFreezeFrame;
  } else if (mode == test_OxygenSensors) {  // Mode 05
    startByte = 6;
    targetArray = supportedOxygenSensor;
  } else if (mode == test_OtherComponents) {  // Mode 06
    startByte = 6;
    targetArray = supportedOtherComponents;
  } else if (mode == control_OnBoardComponents) {  // Mode 08
    startByte = 5;
    targetArray = supportedControlComponents;
  } else if (mode == read_VehicleInfo) {  // Mode 09
    startByte = 6;
    targetArray = supportedVehicleInfo;
  } else {
    return -1;  // Invalid mode
  }

  uint8_t pidCmds[] = {SUPPORTED_PIDS_1_20, SUPPORTED_PIDS_21_40, SUPPORTED_PIDS_41_60, SUPPORTED_PIDS_61_80, SUPPORTED_PIDS_81_100};

  for (int n = 0; n < 5; n++) {
    // Group 0 is always processed, others must be checked
    if (n != 0 && !isInArray(targetArray, 32, pidCmds[n])) break;

    writeData((uint8_t[]){mode, pidCmds[n]});
    if (readData() && resultBuffer[3] == 0x40 + mode) {
      for (int i = 0; i < 4; i++) {
        uint8_t value = resultBuffer[i + startByte];
        for (int bit = 7; bit >= 0; bit--) {
          if ((value >> bit) & 1) targetArray[supportedCount++] = pidIndex + 1;
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
  } else if (mode == 0x05) {
    if (index >= 0) return supportedOxygenSensor[index];
  } else if (mode == 0x06) {
    if (index >= 0) return supportedOtherComponents[index];
  } else if (mode == 0x08) {
    if (index >= 0) return supportedControlComponents[index];
  } else if (mode == 0x09) {
    if (index >= 0) return supportedVehicleInfo[index];
  }
  return 0;
}

// ----------------------------------- Helper Functions -----------------------------------

void OBD2_KLine::updateConnectionStatus(bool messageReceived) {
  if (messageReceived) {
    unreceivedDataCount = 0;
    // if (!connectionStatus) {
    //   connectionStatus = true;
    //   debugPrintln(F("✅ Connection established."));
    // }
  } else {
    if (!connectionStatus) return;  // No need to update if not connected

    unreceivedDataCount++;
    debugPrint(F("⚠️ Not received data: "));
    debugPrintln(String(unreceivedDataCount).c_str());
    if (unreceivedDataCount > 2 && connectionStatus) {
      connectionStatus = false;
      unreceivedDataCount = 0;
      debugPrintln(F("⛔ Connection lost."));
    }
  }
}

void OBD2_KLine::setByteWriteInterval(uint16_t interval) {
  _byteWriteInterval = interval;
}

void OBD2_KLine::setInterByteTimeout(uint16_t interval) {
  _interByteTimeout = interval;
}

void OBD2_KLine::setReadTimeout(uint16_t timeoutMs) {
  _readTimeout = timeoutMs;
}

void OBD2_KLine::setProtocol(const String &protocolName) {
  selectedProtocol = protocolName;
  connectionStatus = false;  // Reset connection status
  connectedProtocol = "";    // Reset connected protocol
  debugPrintln(("Protocol set to: " + selectedProtocol).c_str());
}

// 5 Baud 7O1 (1 start, 7 data, 1 parity, 1 stop)
int OBD2_KLine::read5baud() {
  // debugPrintln(F("Waiting for 5-baud init..."));
  setSerial(false);
  const unsigned long THRESHOLD = 100000;

  // HIGH -> LOW (start bit decrease)
  while (digitalRead(_rxPin) == HIGH);

  // debugPrintln(F("Transition detected. Measuring start bit... "));
  unsigned long tStart = micros();

  while (digitalRead(_rxPin) == LOW) {
    if (micros() - tStart > THRESHOLD) {
      // debugPrintln(F("✅ LOW > 100ms, 5-baud detected"));
      break;
    }
  }

  if (digitalRead(_rxPin) == HIGH && (micros() - tStart <= THRESHOLD)) {
    // debugPrintln(F("❌ No 5 Baud data detected."));
    setSerial(true);
    return -1;
  }

  debugPrint(F("✅ Received 5 Baud data - "));
  uint8_t bits[10];

  delay(200);
  for (int i = 1; i < 10; i++) {
    bits[i] = digitalRead(_rxPin);
    delay(200);
  }

  debugPrint(F("Bits: "));
  for (int i = 0; i < 10; i++) {
    debugPrint(bits[i] ? "1" : "0");
  }

  uint8_t data = 0;
  int ones = 0;
  for (int i = 1; i <= 7; i++) {
    data |= (bits[i] << (i - 1));
    if (bits[i]) ones++;
  }
  if (bits[8]) ones++;

  debugPrint(F(", DATA: 0x"));
  debugPrintHex(data);

  if ((ones & 1) == 0)
    debugPrintln(F(", ❌ Parity ERROR (odd expected)"));
  else
    debugPrintln(F(", ✅ Parity OK"));
  // debugPrintln();
  setSerial(true);

  return data;
}

// 5 Baud 7O1 (1 start, 7 data, 1 parity, 1 stop)
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

  debugPrint(F("➡️ 5 Baud Init for Module 0x"));
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

uint8_t OBD2_KLine::checksum8_XOR(const uint8_t *dataArray, int length) {
  uint8_t checksum = 0;
  for (int i = 0; i < length; i++) {
    checksum ^= dataArray[i];  // XOR operation
  }
  return checksum;
}

uint8_t OBD2_KLine::checksum8_Modulo256(const uint8_t *dataArray, int length) {
  unsigned int sum = 0;
  for (int i = 0; i < length; i++) {
    sum += dataArray[i];
  }
  return (byte)(sum % 256);  // or (byte)sum; because uint8_t overflow also gives a mod 256 effect.
}

uint8_t OBD2_KLine::checksum8_TwosComplement(const uint8_t *dataArray, int length) {
  unsigned int sum = 0;
  for (int i = 0; i < length; i++) {
    sum += dataArray[i];
  }
  byte checksum = (byte)((0x100 - (sum & 0xFF)) & 0xFF);
  return checksum;
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

bool OBD2_KLine::isInArray(const uint8_t *dataArray, uint8_t length, uint8_t value) {
  for (int i = 0; i < length; i++) {
    if (dataArray[i] == value) {
      return true;
    }
  }
  return false;
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

// ----------------------------------- Debug Functions -----------------------------------

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

void OBD2_KLine::setInitAddress(uint8_t address) {
  defaultInitAddress = address;
  // ISO14230 Header'ındaki hedef adresini de otomatik güncellemek isteyebilirsiniz:
  header_ISO14230_Fast[1] = address;
  debugPrint(F("✅ New Init Address set to: "));
  debugPrintHex(address);
  debugPrintln(F(""));
}

void OBD2_KLine::setISO9141Header(uint8_t h1, uint8_t h2, uint8_t h3) {
  header_ISO9141[0] = h1;
  header_ISO9141[1] = h2;
  header_ISO9141[2] = h3;
  debugPrintln(F("✅ ISO9141 Header Updated."));
}

void OBD2_KLine::setISO14230Header(uint8_t h1, uint8_t h2, uint8_t h3) {
  header_ISO14230_Fast[0] = h1;
  header_ISO14230_Fast[1] = h2;
  header_ISO14230_Fast[2] = h3;
  debugPrintln(F("✅ ISO14230 Header Updated."));
}

void OBD2_KLine::setLengthMode(bool inHeader) {
  useLengthInHeader = inHeader;
}