/*
 * OBD2_KLine Library - MukiTech
 * -----------------------------
 * Protocol layer: the shared type vocabulary, the protocol configuration
 * structure, protocol selection, packet building and the connection
 * (handshake) logic.
 *
 * Everything that describes the shape of a packet (header, length byte mode)
 * and everything the handshake needs (init address, init parity, W1..W4 and the
 * wake-up delay) is owned by this class - the core below only moves bytes.
 *
 * The shared enums (OBD2Protocol, OBD2InitType, OBD2Checksum, OBD2LengthMode,
 * OBD2Parity) live at the top of this file. They describe WHAT the bus looks
 * like, so they belong to the protocol layer - but the core, the ECU files and
 * the user sketch all speak this vocabulary, which is why they sit above the
 * class instead of inside it.
 *
 * The default values of every protocol are collected at the top of
 * KLine_Protocol.cpp, one OBD2ProtocolConfig table per protocol.
 * getProtocolConfig() below returns the requested one.
 *
 * The handshakes are shared between protocols (ISO9141 / ISO14230_Slow / KW82
 * all use the same 5 baud init, ISO14230_Fast / UDS use the same fast init),
 * so they are implemented once here and every protocol file only declares
 * WHICH one it uses through its "initType" field.
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

#ifndef KLINE_PROTOCOL_H
#define KLINE_PROTOCOL_H

#include <Arduino.h>

#include "OBD2_KLine_Core.h"

// ==================== Shared Types ====================
// These are defined BEFORE "KLine_Functions.h" is pulled in on purpose: the
// free functions there take an OBD2Checksum, so the enums have to exist first.

// Supported protocols.
//
// Every value here is A DIFFERENT PACKET FORMAT. How the connection is
// established (5 baud / fast / none) is a separate axis chosen with
// setInitType(), and who is being addressed is chosen with setInitAddress() /
// setHeader(). So there is no separate "ISO14230 + fast init" protocol, only a
// combination of three settings:
//
//   obd.setProtocol(ISO14230);      // packet format
//   obd.setInitType(Init_5Baud);    // connection method
//   obd.setHeader(header, 3);       // who is being addressed
enum OBD2Protocol {
  Automatic = 0,  // Tries the known protocols one by one until one connects
  ISO9141 = 1,    // Has a header, no length byte
  ISO14230 = 2,   // = KWP2000. Length is embedded in the first header byte
  KW1281 = 3,     // VAG block protocol (no header / checksum, every byte acknowledged)
  DS2 = 4,        // BMW. Separate length byte (counts the whole frame), XOR checksum
  KW82 = 5,       // Opel. No header, separate length byte
  Custom = 6,     // No preset - the user supplies everything
  None = 7        // No protocol selected
};

// Handshake options (which init method is used).
// Every protocol table has a default; setInitType() overrides it.
enum OBD2InitType {
  Init_None = 0,        // No handshake - the bus is assumed ready to use
  Init_5Baud = 1,       // 5 baud slow init
  Init_5Baud_Block = 2, // 5 baud slow init (block protocols, KW1281)
  Init_Fast = 3,        // Fast init (25ms LOW / 25ms HIGH)
  Init_Ping = 4         // Simple ping packet (DS2)
};

// Checksum options
enum OBD2Checksum {
  Checksum_None = 0,
  Checksum_XOR = 1,
  Checksum_Modulo256 = 2,
  Checksum_TwosComplement = 3
};

// Where the length byte sits in the packet
enum OBD2LengthMode {
  Length_None,          // Like ISO 9141-2 (no length byte)
  Length_SeparateByte,  // Like DS2 / KWP with large payloads (its own length byte)
  Length_InHeader       // Like ISO 14230 (KWP2000) (embedded in the first header byte)
};

// How the parity bit is calculated during 5 baud init
enum OBD2Parity {
  Parity_Even = 0,  // Even parity (ISO9141, ISO14230)
  Parity_Odd = 1    // Odd parity (KW1281, KW82)
};

#include "KLine_Functions.h"  // calculateChecksum(), compareData(), decodeDTC() ... - pure functions

// ==================== Protocol Configuration ====================

// All default settings of one protocol
struct OBD2ProtocolConfig {
  const char* name;          // Protocol name
  uint32_t baudRate;         // Default baud rate
  uint32_t serialConfig;     // Default serial port setting (SERIAL_8N1, SERIAL_8E1 ...)
  OBD2Checksum checksum;     // Default checksum type
  bool verifyChecksum;       // Should incoming data be checksum verified

  // ---- Handshake ----
  OBD2InitType initType;     // Handshake method to use
  // Parity of the 5 baud address byte. Used ONLY while initType is Init_5Baud /
  // Init_5Baud_Block - for fast init and ping protocols send5baud() is never
  // called, so the value is not read (it is still written out in the table).
  OBD2Parity initParity;
  uint8_t initAddress;       // Default init address (0x33 = standard OBD2)
  const uint8_t* header;     // Default header bytes (nullptr if there is none)
  uint8_t headerLength;      // Header length
  int8_t addressIndex;       // Position of the init address inside the header (-1 = none)

  OBD2LengthMode lengthMode;    // Length byte mode
  uint8_t lengthMask;           // Bit mask for Length_InHeader
  bool lengthIncludesFrame;     // Does the length byte count the whole frame (DS2)

  uint16_t p1Time;           // P1: ECU Inter-byte timeout
  uint16_t p2Time;           // P2: ECU Response timeout
  uint16_t p3Time;           // P3: Inter-message gap
  uint16_t p4Time;           // P4: Tester Inter-byte delay
  uint16_t w1Time;           // W1: Address to 0x55
  uint16_t w2Time;           // W2: 0x55 to KW1
  uint16_t w3Time;           // W3: KW1 to KW2
  uint16_t w4Time;           // W4: KW2 to ~KW2
  uint16_t wakeUpDelay;      // Wake-up (Bus Idle) delay
};

// Returns the default settings of the requested protocol (nullptr if unknown)
const OBD2ProtocolConfig* getProtocolConfig(OBD2Protocol protocol);

class KLine_Protocol : public KLine_Core {
 public:
  void setProtocol(OBD2Protocol protocol);
  void setProtocol(uint8_t protocolId);
  OBD2Protocol getConnectedProtocol();
  const char* getProtocolName(OBD2Protocol protocol);

  // ---- Packet layout (for setting things by hand instead of using a preset) ----
  void setHeader(const uint8_t* header, uint8_t length);
  template <size_t N>
  void setHeader(const uint8_t (&headerArray)[N]) {
    setHeader(headerArray, N);
  }
  void setLengthMode(OBD2LengthMode mode);

  // ---- Handshake settings ----
  // The init method is an axis of its own, independent of the protocol: every
  // protocol table has a default and setInitType() overrides it. Because
  // setProtocol() restores that default, setInitType() must ALWAYS be called
  // AFTER setProtocol().
  //   obd.setProtocol(ISO14230);
  //   obd.setInitType(Init_5Baud);   // same protocol, with slow init
  void setInitType(OBD2InitType type);
  void setInitType(uint8_t typeId);
  OBD2InitType getInitType();
  const char* getInitTypeName(OBD2InitType type);

  void setInitAddress(uint8_t address);
  void setInitParity(OBD2Parity parity);  // 5 baud init parity (Parity_Even / Parity_Odd)
  void setWakeUpDelay(uint16_t delayMs);

  bool connect();
  bool trySlowInit();
  bool tryFastInit();

  // ---- Checksum policy ----
  // The arithmetic itself lives in KLine_Functions.h (pure functions).
  // Which type is active and whether incoming data gets verified are decisions
  // that belong to the packet, so they are made here.
  void setChecksumType(OBD2Checksum checksum);
  void setChecksumType(uint8_t checksumId);
  void setChecksumVerify(bool enabled);
  bool verifyChecksum(const uint8_t* data, uint8_t length);

  // Adds no header, only appends the requested checksum and sends.
  void writeRawData(const uint8_t* dataArray, uint8_t length, OBD2Checksum checksum = Checksum_None);

  template <size_t N>
  void writeRawData(const uint8_t (&dataArray)[N], OBD2Checksum checksum = Checksum_None) {
    writeRawData(dataArray, N, checksum);
  }

  // ---- Reading and connection status ----
  // The core only reads bytes. Checksum verification and the "am I connected"
  // decision are made here; when a subclass calls readData() both happen
  // automatically.
  uint8_t readData();

  bool isConnected();
  void updateConnectionStatus(bool messageReceived);
  void setConnectionStatus(bool status);
  void setMaxRetryCount(uint8_t count);  // 0 = disabled (the connection never drops)

  // Builds the packet according to the active protocol settings (header, length
  // byte, checksum) into "frame" and returns its length.
  // "frame" must be at least dataLength + MAX_FRAME_OVERHEAD bytes long.
  static const uint8_t MAX_FRAME_OVERHEAD = 8; // max header(6) + length byte(1) + checksum(1)
  uint8_t buildFrame(const uint8_t* data, uint8_t dataLength, uint8_t* frame);

  // ====== THE THREE WAYS TO WRITE ======
  //
  //  writeData(...)     You supply only the data bytes.
  //                     Header + length + checksum are added by the LIBRARY.
  //                       writeData((uint8_t[]){0x21, 0x01});
  //                       -> 80 11 F1 02 21 01 <cs>
  //
  //  writeRawData(...)  No header is added, only the requested checksum is
  //                     appended at the end.
  //                       writeRawData(packet, length, Checksum_Modulo256);
  //
  //  sendBytes(...)     Nothing is added. Whatever you pass goes on the bus as
  //                     it is - you write the header and the checksum yourself.
  //                     (in KLine_Core)
  //                       sendBytes((uint8_t[]){0x80,0x11,0xF1,0x02,0x21,0x01,0x45}, 7);

  // Prepares the packet and hands it to the core to be sent
  void writeData(const uint8_t* data, uint8_t dataLength);

  template <size_t N>
  void writeData(const uint8_t (&dataArray)[N]) {
    writeData(dataArray, N);
  }

  // Block framing (KW1281): length byte + message counter, every byte is
  // acknowledged with its complement. Uses the core writeByte / readByte.
  void writeBlock(const uint8_t* dataArray, uint8_t length);
  uint8_t readBlock();

 protected:
  // The settings of a protocol split into two halves: what the handshake needs
  // and what the packet looks like. The 5 baud step of Automatic mode loads
  // these at SEPARATE times - first the handshake runs, then the protocol is
  // recognised from the keywords, and only after that do the packet settings
  // follow. Details: KLine_Protocol.cpp / _trySlowInitAuto()
  void applyProtocolPresets(OBD2Protocol protocol);  // both at once
  void applyHandshakeSettings(const OBD2ProtocolConfig* config);
  void applyFrameSettings(const OBD2ProtocolConfig* config);

  bool _tryProtocol(OBD2Protocol protocol, OBD2InitType init);
  bool _trySlowInitAuto();  // Automatic mode: the 5 baud step that finds the protocol

  // ---- Packet layout (buildFrame uses these) ----
  uint8_t _header[6];        // Support for a header of up to 6 bytes
  uint8_t _headerLength = 0; // Current header length
  OBD2LengthMode _lengthMode = Length_None;
  uint8_t _lengthMask = 0x3F; // Length bit mask for ISO 14230
  bool _lengthIncludesFrame = false; // Protocols where the length counts the whole frame, like DS2

  // ---- Checksum policy ----
  OBD2Checksum checksumType = Checksum_Modulo256;
  bool _verifyChecksum = true;

  // ---- Connection status ----
  bool connectionStatus = false;
  uint8_t unreceivedDataCount = 0;
  uint8_t _maxRetryCount = 3;

  // ---- Protocol status ----
  OBD2Protocol selectedProtocol = Automatic; // What the user selected (may be Automatic)
  OBD2Protocol connectedProtocol = None;     // What was actually connected to after the handshake
  OBD2Protocol currentProtocol = None;       // The protocol currently in use (being tried or connected)

  // ---- Handshake settings and timings ----
  // The init method currently in effect. applyProtocolPresets() writes the
  // protocol default here, setInitType() overrides it, _tryProtocol() reads it.
  OBD2InitType _initType = Init_None;
  uint8_t defaultInitAddress = 0x33;
  bool _isOddParity = false;      // Handshake parity (Odd vs Even)
  uint16_t _w1Time = 300;         // W1: Address to 0x55 (Max 300ms)
  uint16_t _w2Time = 20;          // W2: 0x55 to KW1 (Max 20ms)
  uint16_t _w3Time = 20;          // W3: KW1 to KW2 (Max 20ms)
  uint16_t _w4Time = 30;          // W4: KW2 to ~KW2 (Max 50ms)
  uint16_t _wakeUpDelayMs = 5500; // Wake-up (Bus Idle) delay

  uint8_t blockMessageCount = 0;  // KW1281 block counter
};

#endif  // KLINE_PROTOCOL_H
