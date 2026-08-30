/*
 * OBD2_KLine Library - MukiTech
 * -----------------------------
 * K-Line core: microcontroller / serial port handling, sending and receiving
 * bytes, the raw init signals and the debug output.
 *
 * This class knows no protocols and no diagnostic services. It only produces
 * the electrical signals (5 baud pattern, fast init pulse) and moves bytes -
 * sendBytes() transmits an already prepared packet as it is. Building that
 * packet and deciding which signal is used when is the protocol layer's job
 * (see "KLine_Protocol.h").
 *
 * So everything that describes a PACKET (header, length byte, init address,
 * init parity) or a HANDSHAKE (W1..W4, wake-up delay) lives in KLine_Protocol,
 * not here. What stays here is what the microcontroller itself needs: the port,
 * the pins, the P1..P4 byte timings, the shared helper functions used while sending
 * and receiving (see "KLine_Functions.h"), and the debug output.
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

#ifndef OBD2_KLINE_CORE_H
#define OBD2_KLINE_CORE_H

#include <Arduino.h>

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
#include <AltSoftSerial.h>
#endif

#if defined(__AVR__) // || defined(ESP8266) || defined(ESP32)
#include <SoftwareSerial.h>
#endif

class KLine_Core {
 public:
  KLine_Core();
  void setSerial(HardwareSerial& serial);
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  void setSerial(AltSoftSerial& serial);
#endif
#if defined(__AVR__) // || defined(ESP8266) || defined(ESP32)
  void setSerial(SoftwareSerial& serial);
#endif

  void setPins(uint8_t rx, uint8_t tx);
  void setBaudRate(uint32_t baud);
  void setSerialConfig(uint32_t config); // e.g. SERIAL_8E1

  void setDebug(Stream& serial);
  void toggleSerial(bool state);

  void begin();
  void begin(uint32_t baud, uint32_t config = SERIAL_8N1, int16_t rx = -1, int16_t tx = -1);

  // Puts a ready made packet on the bus as it is - neither a header nor a
  // checksum is added. Use this when you want to write every byte yourself.
  void sendBytes(const uint8_t* dataArray, uint8_t length);

  template <size_t N>
  void sendBytes(const uint8_t (&dataArray)[N]) {
    sendBytes(dataArray, N);
  }

  // Reads the bytes coming from the serial port and returns how many were
  // read. That is all - the "am I connected" decision is not made here, that
  // is the job of KLine_Protocol.
  uint8_t readData();

  // Hardware level init signals (the sequences that use them live in the protocol layer)
  void send5baud(uint8_t data, bool isOddParity = false);
  int read5baud();
  void sendFastInitPulse(); // 25ms LOW + 25ms HIGH wake-up pattern

  // Single byte access (assembling a block is the protocol layer's job)
  void writeByte(uint8_t data);
  int readByte();

  // Byte level timing (used while sending / receiving - the handshake timings
  // W1..W4 and the wake-up delay belong to the protocol layer)
  void setP1Time(uint16_t timeMs); // ECU Inter-byte timeout (Max time between ECU bytes)
  void setP2Time(uint16_t timeMs); // ECU Response timeout (Max wait for ECU to start responding)
  void setP3Time(uint16_t timeMs); // Inter-message gap (Min time to wait before next request)
  void setP4Time(uint16_t timeMs); // Tester Inter-byte delay (Gap between bytes sent to ECU)

  // Data Access Functions
  uint8_t* getResultBuffer();
  uint8_t getResultLength();

 protected:
  Stream* _debugSerial = nullptr;  // Debug serial port

  Stream* _serial = nullptr;
  HardwareSerial* _hwSerial = nullptr;
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  AltSoftSerial* _altSerial = nullptr;
#endif
#if defined(__AVR__) //|| defined(ESP8266) || defined(ESP32)
  SoftwareSerial* _softSerial = nullptr;
#endif
  uint32_t _baudRate = 10400;
  int16_t _rxPin = 2;
  int16_t _txPin = 3;
  uint32_t _serialConfig = SERIAL_8N1;

  uint8_t resultBuffer[300] = {0};

  bool _isSerialEnabled = false;           // Hardware state of the serial port
  uint8_t _lastReadLength = 0;
  uint16_t _p1Time = 20;   // P1: ECU Inter-byte timeout
  uint16_t _p2Time = 1000; // P2: ECU Response timeout
  uint16_t _p3Time = 57;   // P3: Inter-message gap
  uint16_t _p4Time = 5;    // P4: Tester Inter-byte delay
  unsigned long _lastResponseTime = 0;

  void clearEcho(uint8_t length);
  void debugPrint(const char* msg);
  void debugPrint(const __FlashStringHelper* msg);
  void debugPrint(uint32_t val);
  void debugPrintln(const char* msg);
  void debugPrintln(const __FlashStringHelper* msg);
  void debugPrintln(uint32_t val);
  void debugPrintHex(uint8_t val);    // Hexadecimal output
  void debugPrintHexln(uint8_t val);  // Hexadecimal + newline
};

#endif  // OBD2_KLINE_CORE_H
