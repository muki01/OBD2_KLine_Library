/*
 * OBD2_KLine Library - MukiTech
 * -----------------------------
 * K-Line core implementation: serial port / pin handling, raw send - receive,
 * checksum arithmetic, the raw init signals and the debug output.
 * Packet layout (header, length byte) and handshake sequences are not here -
 * they belong to KLine_Protocol.cpp.
 *
 * Developed by: Muksin Muksin (MukiTech)
 * GitHub: https://github.com/muki01/OBD2_KLine_Library
 * Email: muksin.muksin04@gmail.com
 *
 * LICENSE: DUAL-LICENSED
 * 1. PERSONAL/RESEARCH: Free for non-commercial use.
 * 2. COMMERCIAL: Mandatory paid license required for any for-profit usage.
 * Copyright (c) 2025 MukiTech. All rights reserved.
 */

#include "OBD2_KLine_Core.h"

KLine_Core::KLine_Core()
    : _baudRate(10400), _rxPin(2), _txPin(3), _serialConfig(SERIAL_8N1) {
}

void KLine_Core::setSerial(HardwareSerial& serial) {
  _hwSerial = &serial;
  _serial = &serial;
}

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
void KLine_Core::setSerial(AltSoftSerial& serial) {
  _altSerial = &serial;
  _serial = &serial;
}
#endif

#if defined(__AVR__)
void KLine_Core::setSerial(SoftwareSerial& serial) {
  _softSerial = &serial;
  _serial = &serial;
}
#endif

void KLine_Core::setPins(uint8_t rx, uint8_t tx) {
  _rxPin = rx;
  _txPin = tx;
  debugPrint(F("✅ Pins updated to: RX=")); debugPrint(rx); debugPrint(F(", TX=")); debugPrintln(tx);
}

void KLine_Core::setBaudRate(uint32_t baud) {
  _baudRate = baud;
  debugPrint(F("✅ Baud rate updated to: ")); debugPrintln(baud);
}

// Constants like SERIAL_8N1 are COMPLETELY different numbers per platform:
// 0x06 on AVR, 0x800001C (= 134217756) on ESP32. The number on its own tells
// nobody anything, so the name is printed to the log as well.
static const char* serialConfigName(uint32_t config) {
#ifdef SERIAL_8N1
  if (config == SERIAL_8N1) return "8N1";
#endif
#ifdef SERIAL_8E1
  if (config == SERIAL_8E1) return "8E1";
#endif
#ifdef SERIAL_8O1
  if (config == SERIAL_8O1) return "8O1";
#endif
#ifdef SERIAL_8N2
  if (config == SERIAL_8N2) return "8N2";
#endif
#ifdef SERIAL_7E1
  if (config == SERIAL_7E1) return "7E1";
#endif
  return nullptr;  // Unknown combination - only the raw value is printed
}

void KLine_Core::setSerialConfig(uint32_t config) {
  _serialConfig = config;

  const char* name = serialConfigName(config);
  debugPrint(F("✅ Serial config updated to: "));
  if (name != nullptr) {
    debugPrint(name);
    debugPrint(F(" ("));
    debugPrint(config);
    debugPrintln(F(")"));
  } else {
    debugPrintln(config);
  }
}

// ----------------------------------- Initialization functions -----------------------------------

void KLine_Core::toggleSerial(bool state) {
  if (_serial == nullptr) return;

  if (state == true) {
    if (_isSerialEnabled == true){
      debugPrintln(F("🔄 Restarting serial..."));
      if (_hwSerial != nullptr) _hwSerial->end();
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
      else if (_altSerial != nullptr) _altSerial->end();
#endif
#if defined(__AVR__)
      else if (_softSerial != nullptr) _softSerial->end();
#endif
      debugPrintln(F("❌ Serial disabled."));
      _isSerialEnabled = false;
      pinMode(_rxPin, INPUT);
      pinMode(_txPin, OUTPUT);
      digitalWrite(_txPin, HIGH);
    }

    debugPrint(F("🔄 Starting serial..."));
    if (_hwSerial != nullptr) {
#if defined(ESP32)
      _hwSerial->begin(_baudRate, _serialConfig, _rxPin, _txPin);
#else
      _hwSerial->begin(_baudRate, _serialConfig);
#endif
    }
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
    else if (_altSerial != nullptr) {
      _altSerial->begin(_baudRate);
    }
#endif
#if defined(__AVR__)
    else if (_softSerial != nullptr) {
      _softSerial->begin(_baudRate);
    }
#endif
    debugPrintln(F("✅ Serial enabled."));
    _isSerialEnabled = true;
  } else {
    debugPrint(F("🔄 Stopping serial..."));
    if (_isSerialEnabled == false) {
      debugPrintln(F("❌ Serial disabled."));
      return;
    }
    if (_hwSerial != nullptr) _hwSerial->end();
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
    else if (_altSerial != nullptr) _altSerial->end();
#endif
#if defined(__AVR__)
    else if (_softSerial != nullptr) _softSerial->end();
#endif
    debugPrintln(F("❌ Serial disabled."));
    _isSerialEnabled = false;
    pinMode(_rxPin, INPUT);
    pinMode(_txPin, OUTPUT);
    digitalWrite(_txPin, HIGH);
  }
}

void KLine_Core::begin() {
  toggleSerial(true);
}

void KLine_Core::begin(uint32_t baud, uint32_t config, int16_t rx, int16_t tx) {
  _baudRate = baud;
  _serialConfig = config;
  if (rx != -1) _rxPin = rx;
  if (tx != -1) _txPin = tx;
  begin();
}


// ----------------------------------- Basic Read/Write functions -----------------------------------

// Puts a ready made packet on the bus as it is. How that packet was prepared
// (header, length byte, checksum) is the protocol layer's job - this only sends.
void KLine_Core::sendBytes(const uint8_t* dataArray, uint8_t length) {
  while (millis() - _lastResponseTime < _p3Time) {
    yield();
  }

  debugPrint(F("\n➡️ Sending Data: "));
  for (size_t i = 0; i < length; i++) {
    debugPrintHex(dataArray[i]);
    debugPrint(F(" "));
  }
  debugPrintln(F(""));

  for (size_t i = 0; i < length; i++) {
    _serial->write(dataArray[i]);
    if (i < length - 1) delay(_p4Time);
  }

  clearEcho(length);
}

void KLine_Core::writeByte(uint8_t data) {
  delay(_p4Time);
  _serial->write(data);
  clearEcho(1);
}

int KLine_Core::readByte() {
  unsigned long startMillis = millis();

  while (millis() - startMillis < _p2Time) {
    if (_serial->available() > 0) {
      return _serial->read();
    }
  }

  debugPrintln(F("❌ Timeout: Not Received Data."));
  return -1;
}

uint8_t KLine_Core::readData() {
  debugPrint(F("Reading Data ... "));
  unsigned long startMillis = millis();
  int bytesRead = 0;

  // Wait for data for the specified timeout
  while (millis() - startMillis < _p2Time) {
    if (_serial->available() > 0) {
      unsigned long lastByteTime = millis();
      memset(resultBuffer, 0, sizeof(resultBuffer));

      // Read all data
      debugPrint(F("✅ Received Data: "));
      while (millis() - lastByteTime < _p1Time) {  // Wait for new data
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


      // debugPrintln(F("\n✅ Data reception completed."));

      _lastReadLength = bytesRead;
      _lastResponseTime = lastByteTime;
      debugPrintln(F(""));

      return bytesRead;
    }
  }

  // If no data is received within 1 second
  debugPrintln(F("❌ OBD2 Timeout!"));
  _lastReadLength = 0;
  return 0;
}

void KLine_Core::clearEcho(uint8_t length) {
  const unsigned long byteTimeoutMs = 100;

  // Wait for the first byte
  unsigned long startTime = millis();
  while (_serial->available() == 0) {
    if (millis() - startTime >= byteTimeoutMs) {
      debugPrintln(F("❌ Echo not received"));
      return;
    }
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
    }

    readedByte = _serial->read();
    debugPrintHex(readedByte);
    debugPrint(F(" "));
  }

  debugPrintln(F(""));
}

uint8_t* KLine_Core::getResultBuffer() {
  return resultBuffer;
}

uint8_t KLine_Core::getResultLength() {
  return _lastReadLength;
}

// ----------------------------------- Timing -----------------------------------

void KLine_Core::setP1Time(uint16_t timeMs) {
  _p1Time = timeMs;
  debugPrint(F("✅ P1 Time set to: "));
  debugPrint(timeMs);
  debugPrintln(F(" ms"));
}

void KLine_Core::setP2Time(uint16_t timeMs) {
  _p2Time = timeMs;
  debugPrint(F("✅ P2 Time set to: "));
  debugPrint(timeMs);
  debugPrintln(F(" ms"));
}

void KLine_Core::setP3Time(uint16_t timeMs) {
  _p3Time = timeMs;
  debugPrint(F("✅ P3 Time set to: "));
  debugPrint(timeMs);
  debugPrintln(F(" ms"));
}

void KLine_Core::setP4Time(uint16_t timeMs) {
  _p4Time = timeMs;
  debugPrint(F("✅ P4 Time set to: "));
  debugPrint(timeMs);
  debugPrintln(F(" ms"));
}

// ----------------------------------- 5 Baud Init -----------------------------------

// 5 Baud 7O1 (1 start, 7 data, 1 parity, 1 stop)
int KLine_Core::read5baud() {
  // debugPrintln(F("Waiting for 5-baud init..."));
  toggleSerial(false);
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
    toggleSerial(true);
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
  toggleSerial(true);

  return data;
}

// 5 Baud (1 start, 7 data, 1 parity, 1 stop)
void KLine_Core::send5baud(uint8_t data, bool isOddParity) {
  uint8_t bits[10];
  bits[0] = 0;  // start bit
  bits[9] = 1;  // stop bit

  // 7-bit data
  for (int i = 0; i < 7; i++) {
    bits[i + 1] = (data >> i) & 1;
  }

  // Parity calculation
  uint8_t ones = 0;
  for (int i = 1; i <= 7; i++) {
    if (bits[i]) ones++;
  }

  if (isOddParity) {
    bits[8] = (ones % 2 == 0) ? 1 : 0;  // odd parity bit
  } else {
    bits[8] = (ones % 2 == 0) ? 0 : 1;  // even parity (was default in original code)
  }

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

// Fast init wake-up pattern (25ms LOW / 25ms HIGH) on the K-Line.
// Only the pulse itself is here; which protocol uses it and when is the
// protocol layer's job. (Must be called while the serial port is closed.)
void KLine_Core::sendFastInitPulse() {
  digitalWrite(_txPin, LOW); delay(25);
  digitalWrite(_txPin, HIGH); delay(25);
}

// ----------------------------------- Debug Functions -----------------------------------

void KLine_Core::setDebug(Stream& serial) {
  _debugSerial = &serial;
}

void KLine_Core::debugPrint(const char* msg) {
  if (_debugSerial) _debugSerial->print(msg);
}

void KLine_Core::debugPrint(const __FlashStringHelper* msg) {
  if (_debugSerial) _debugSerial->print(msg);
}

void KLine_Core::debugPrint(uint32_t val) {
  if (_debugSerial) _debugSerial->print(val);
}

void KLine_Core::debugPrintln(const char* msg) {
  if (_debugSerial) _debugSerial->println(msg);
}

void KLine_Core::debugPrintln(const __FlashStringHelper* msg) {
  if (_debugSerial) _debugSerial->println(msg);
}

void KLine_Core::debugPrintln(uint32_t val) {
  if (_debugSerial) _debugSerial->println(val);
}

void KLine_Core::debugPrintHex(uint8_t val) {
  if (_debugSerial) {
    if (val < 0x10) _debugSerial->print("0");
    _debugSerial->print(val, HEX);
  }
}

void KLine_Core::debugPrintHexln(uint8_t val) {
  if (_debugSerial) {
    debugPrintHex(val);
    _debugSerial->println();
  }
}
