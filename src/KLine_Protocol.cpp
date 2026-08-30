/*
 * OBD2_KLine Library - MukiTech
 * -----------------------------
 * Protocol layer implementation: the default settings of every protocol,
 * applying them and performing the handshakes. Only the raw signals (5 baud
 * pattern, fast init pulse) and the byte level send / receive come from
 * KLine_Core.
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

#include "KLine_Protocol.h"

// ==================================================================================
//                              PROTOCOL DEFAULTS
// ==================================================================================
// All default settings of every protocol are here. To add a new protocol:
// 1) Add a new <PROTOCOL>_CONFIG table below
// 2) Add the new protocol to the OBD2Protocol enum (KLine_Protocol.h)
// 3) Add a single line to the getProtocolConfig() switch at the bottom
// ==================================================================================

// ----------------------------------- ISO 9141-2 -----------------------------------
// 10400 baud, 8N1, 5 baud slow init, 3 byte header (68 6A F1),
// no length byte, Modulo256 checksum.
static const uint8_t ISO9141_HEADER[] = {0x68, 0x6A, 0xF1};

static const OBD2ProtocolConfig ISO9141_CONFIG = {
    "ISO9141",           // name
    10400,               // baudRate
    SERIAL_8N1,          // serialConfig
    Checksum_Modulo256,  // checksum
    true,                // verifyChecksum

    Init_5Baud,      // initType
    Parity_Even,     // initParity (5 baud)
    0x33,            // initAddress
    ISO9141_HEADER,  // header
    3,               // headerLength
    -1,              // addressIndex (the header carries no address)

    Length_None,  // lengthMode
    0x3F,         // lengthMask
    false,        // lengthIncludesFrame

    20,    // P1
    1000,  // P2
    57,    // P3
    5,     // P4
    300,   // W1
    30,    // W2
    20,    // W3
    30,    // W4
    5500   // wakeUpDelay
};

// ----------------------------- ISO 14230 (KWP2000) -----------------------------
// 10400 baud, 8N1, 3 byte header, length embedded in the first header byte,
// Modulo256 checksum.
//
// Byte 1 of the header is the target address and gets replaced by the init
// address (addressIndex = 1). The default is 0x33 (the functional OBD2
// address); setInitAddress() can turn it into a physical ECU address (0x01).
//
// Byte 0 of the header is the addressing mode (upper 2 bits; lower 6 bits are
// the length):
//   0xC0 = functional (default, standard OBD2)
//   0x80 = physical    -> selected with setHeader((uint8_t[]){0x80, 0x33, 0xF1})
//
// The init method is NOT FIXED in this table: the Init_Fast below is only the
// default, setInitType(Init_5Baud) turns it into slow init.
static const uint8_t KWP2000_HEADER[] = {0xC0, 0x33, 0xF1};

static const OBD2ProtocolConfig ISO14230_CONFIG = {
    "ISO14230",          // name
    10400,               // baudRate
    SERIAL_8N1,          // serialConfig
    Checksum_Modulo256,  // checksum
    true,                // verifyChecksum

    Init_Fast,       // initType (the DEFAULT - setInitType() overrides it)
    Parity_Even,     // initParity (used if Init_5Baud is selected)
    0x33,            // initAddress
    KWP2000_HEADER,  // header
    3,               // headerLength
    1,               // addressIndex (byte 1 of the header is the target address)

    Length_InHeader,  // lengthMode
    0x3F,             // lengthMask
    false,            // lengthIncludesFrame

    20,    // P1
    1000,  // P2
    57,    // P3
    5,     // P4
    300,   // W1
    30,    // W2
    20,    // W3
    30,    // W4
    5500   // wakeUpDelay
};

// ----------------------------------- VAG KW1281 -----------------------------------
// 9600 baud, 8N1, 5 baud slow init with odd parity, block communication
// (no header, no checksum - every byte is acknowledged both ways).
static const OBD2ProtocolConfig KW1281_CONFIG = {
    "KW1281",          // name
    9600,              // baudRate
    SERIAL_8N1,        // serialConfig
    Checksum_None,     // checksum
    false,             // verifyChecksum

    Init_5Baud,        // initType (block framing comes from the protocol, not from here)
    Parity_Odd,        // initParity (5 baud)
    0x01,     // initAddress (Engine ECU)
    nullptr,  // header (none)
    0,        // headerLength
    -1,       // addressIndex

    Length_None,  // lengthMode
    0x3F,         // lengthMask
    false,        // lengthIncludesFrame

    20,    // P1
    1000,  // P2
    57,    // P3
    2,     // P4
    300,   // W1
    30,    // W2
    20,    // W3
    30,    // W4
    5500   // wakeUpDelay
};

// ------------------------------------ BMW DS2 ------------------------------------
// 9600 baud, 8E1, no handshake (a simple ping), 1 byte header (the ECU
// address), separate length byte (counts the whole frame), XOR checksum.
// The first header byte is replaced by the init address (addressIndex = 0)
static const uint8_t DS2_HEADER[] = {0x12};

static const OBD2ProtocolConfig DS2_CONFIG = {
    "DS2",          // name
    9600,           // baudRate
    SERIAL_8E1,     // serialConfig
    Checksum_XOR,   // checksum
    true,           // verifyChecksum

    Init_Ping,      // initType
    Parity_Even,    // initParity (unused - there is no handshake)
    0x12,        // initAddress (DME)
    DS2_HEADER,  // header
    1,           // headerLength
    0,           // addressIndex (byte 0 of the header is the init address)

    Length_SeparateByte,  // lengthMode
    0x3F,                 // lengthMask
    true,                 // lengthIncludesFrame (the length counts the whole frame)

    20,    // P1
    1000,  // P2
    57,    // P3
    5,     // P4
    300,   // W1
    20,    // W2
    20,    // W3
    30,    // W4
    5500   // wakeUpDelay
};

// ----------------------------------- Opel KW82 -----------------------------------
// 4800 baud, 8N1, 5 baud slow init with odd parity, no header,
// separate length byte, Modulo256 checksum.
static const OBD2ProtocolConfig KW82_CONFIG = {
    "KW82",              // name
    4800,                // baudRate
    SERIAL_8N1,          // serialConfig
    Checksum_Modulo256,  // checksum
    false,               // verifyChecksum

    Init_5Baud,   // initType
    Parity_Odd,   // initParity (5 baud)
    0x60,     // initAddress
    nullptr,  // header (none)
    0,        // headerLength
    -1,       // addressIndex

    Length_SeparateByte,  // lengthMode
    0x3F,                 // lengthMask
    false,                // lengthIncludesFrame

    10,    // P1
    1000,  // P2
    10,    // P3
    5,     // P4
    300,   // W1
    60,    // W2
    20,    // W3
    30,    // W4
    5500   // wakeUpDelay
};

// ----------------------------------- Protocol Registry -----------------------------------

const OBD2ProtocolConfig* getProtocolConfig(OBD2Protocol protocol) {
  switch (protocol) {
    case ISO9141: return &ISO9141_CONFIG;
    case ISO14230: return &ISO14230_CONFIG;
    case KW1281: return &KW1281_CONFIG;
    case DS2: return &DS2_CONFIG;
    case KW82: return &KW82_CONFIG;
    default: return nullptr;  // Automatic, Custom, None
  }
}

// ----------------------------------- Auto Detect Order -----------------------------------
// Automatic mode makes two attempts and THE TWO ARE NOT THE SAME THING:
//
//   1) Fast init  -> only ISO14230 speaks it, so the protocol is known up front.
//                    Tried first because it is the most common on modern cars.
//   2) Slow init  -> the handshake is IDENTICAL for ISO9141 and ISO14230. Which
//                    one it is only becomes known after the ECU sends its
//                    keywords, so this round is not run on behalf of a protocol.
//                    See _trySlowInitAuto().
//
// That is why there is no "protocol list" table here: the second step is not a
// protocol attempt, it is the step that FINDS the protocol.

// Which table the slow init handshake settings are read from.
// In the ISO9141 and ISO14230 tables these values (baud, parity, W1..W4,
// P1..P4) are identical; where they differ is the header / length / checksum,
// and those are loaded after the handshake. They are read from the table
// instead of being hard coded so the timings can be changed in one place.
static const OBD2Protocol SLOW_INIT_HANDSHAKE_SOURCE = ISO9141;

// ----------------------------------- Protocol Selection -----------------------------------

void KLine_Protocol::setProtocol(OBD2Protocol protocol) {
  selectedProtocol = protocol;
  connectionStatus = false;
  connectedProtocol = None;
  currentProtocol = protocol;

  if(protocol != None && protocol != Automatic && protocol != Custom) applyProtocolPresets(protocol);

  if (_isSerialEnabled) begin();

  debugPrint(F("✅ Protocol set to: "));
  debugPrintln(getProtocolName(selectedProtocol));
}

void KLine_Protocol::setProtocol(uint8_t protocolId) {
  setProtocol((OBD2Protocol)protocolId);
}

OBD2Protocol KLine_Protocol::getConnectedProtocol() {
  return connectedProtocol;
}

const char* KLine_Protocol::getProtocolName(OBD2Protocol protocol) {
  const OBD2ProtocolConfig* config = getProtocolConfig(protocol);
  if (config != nullptr) return config->name;

  switch (protocol) {
    case Custom: return "Custom";
    case None: return "None";
    default: return "Automatic";
  }
}

// All default settings come from the protocol tables at the top of this file.
//
// The settings are split into TWO HALVES, and the split is not arbitrary:
//
//   applyHandshakeSettings()  what is needed UNTIL the handshake FINISHES
//                             (baud, parity, init method, W1..W4, P1..P4)
//   applyFrameSettings()      what is needed AFTER the handshake has finished
//                             (header, length mode, checksum)
//
// ISO9141 and ISO14230 are IDENTICAL in the first half (10400, 8N1, address
// $33, even parity, same timings) - where they differ is the second half. In
// Automatic mode this is what lets the handshake run without a protocol first;
// the incoming keywords then name the protocol and the second half is loaded
// ONLY AFTER that. See _trySlowInitAuto().
void KLine_Protocol::applyProtocolPresets(OBD2Protocol protocol) {
  const OBD2ProtocolConfig* config = getProtocolConfig(protocol);
  if (config == nullptr) return;  // Automatic / Custom / None

  debugPrint(F("✅ Applying protocol presets for: "));
  debugPrintln(config->name);

  applyHandshakeSettings(config);
  applyFrameSettings(config);
}

// What the handshake itself needs.
void KLine_Protocol::applyHandshakeSettings(const OBD2ProtocolConfig* config) {
  if (config == nullptr) return;

  setBaudRate(config->baudRate);
  setSerialConfig(config->serialConfig);
  setInitType(config->initType);  // the protocol DEFAULT (setInitType() can override it)
  setInitParity(config->initParity);

  setP1Time(config->p1Time);
  setP2Time(config->p2Time);
  setP3Time(config->p3Time);
  setP4Time(config->p4Time);

  _w1Time = config->w1Time;
  _w2Time = config->w2Time;
  _w3Time = config->w3Time;
  _w4Time = config->w4Time;
  _wakeUpDelayMs = config->wakeUpDelay;
}

// The shape of the packet: these are only used after the handshake has
// finished, on data requests. If the header carries an address, the current
// init address is written into it, so this function must be called AFTER any
// address changes.
void KLine_Protocol::applyFrameSettings(const OBD2ProtocolConfig* config) {
  if (config == nullptr) return;

  setChecksumType(config->checksum);
  setChecksumVerify(config->verifyChecksum);

  if (config->headerLength > 0 && config->header != nullptr) {
    uint8_t header[6];
    memcpy(header, config->header, config->headerLength);
    if (config->addressIndex >= 0 && config->addressIndex < config->headerLength) {
      header[config->addressIndex] = defaultInitAddress;
    }
    setHeader(header, config->headerLength);
  } else {
    setHeader(nullptr, 0);
  }

  setLengthMode(config->lengthMode);
  _lengthMask = config->lengthMask;
  _lengthIncludesFrame = config->lengthIncludesFrame;
}

// ----------------------------------- Checksum Policy -----------------------------------

void KLine_Protocol::setChecksumType(OBD2Checksum checksum) {
  checksumType = checksum;
  debugPrint(F("✅ Checksum type set to: "));
  debugPrintln(getChecksumName(checksumType));
}

void KLine_Protocol::setChecksumType(uint8_t checksumId) {
  setChecksumType((OBD2Checksum)checksumId);
}

void KLine_Protocol::setChecksumVerify(bool enabled) {
  _verifyChecksum = enabled;
}

bool KLine_Protocol::verifyChecksum(const uint8_t* data, uint8_t length) {
  if (checksumType == Checksum_None || currentProtocol == None) return true;
  if (length < 2) return false;

  uint8_t expected = calculateChecksum(data, length - 1, checksumType);
  return (data[length - 1] == expected);
}

// No header is added, only the checksum is appended.
void KLine_Protocol::writeRawData(const uint8_t* dataArray, uint8_t length, OBD2Checksum checksum) {
  uint8_t checksumLength = (checksum == Checksum_None) ? 0 : 1;
  uint8_t fullDataLength = length + checksumLength;
  uint8_t sendData[fullDataLength];

  memcpy(sendData, dataArray, length);
  if (checksum != Checksum_None) {
    sendData[fullDataLength - 1] = calculateChecksum(dataArray, length, checksum);
  }

  sendBytes(sendData, fullDataLength);
}

// ----------------------------------- Read / Connection State -----------------------------------
// The core only reads bytes from the serial port and reports how many arrived.
// Whether the packet is valid (checksum) and whether the connection is still
// alive are decided here.

uint8_t KLine_Protocol::readData() {
  uint8_t length = KLine_Core::readData();

  if (length > 0 && _verifyChecksum) {
    if (!verifyChecksum(resultBuffer, length)) {
      debugPrintln(F("❌ Checksum Error! Data discarded."));
      _lastReadLength = 0;
      length = 0;
    } else {
      debugPrintln(F("  ✅ Checksum OK"));
    }
  }

  updateConnectionStatus(length > 0);
  return length;
}

bool KLine_Protocol::isConnected() {
  return connectionStatus;
}

void KLine_Protocol::updateConnectionStatus(bool success) {
  if (!connectionStatus) return;  // If we are not connected there is nothing to count

  if (success) {
    unreceivedDataCount = 0;  // A response arrived, reset the counter
  } else {
    // Drop detection only happens while maxRetryCount > 0
    if (_maxRetryCount > 0) {
      unreceivedDataCount++;
      if (unreceivedDataCount >= _maxRetryCount) {
        connectionStatus = false;
        unreceivedDataCount = 0;
        debugPrintln(F("⛔ Critical: Connection lost after multiple retries."));
      }
    }
  }
}

void KLine_Protocol::setConnectionStatus(bool status) {
  connectionStatus = status;
}

void KLine_Protocol::setMaxRetryCount(uint8_t count) {
  _maxRetryCount = count;
  debugPrint(F("✅ Max Retry Count set to: "));
  if (count == 0) {
    debugPrintln(F("Disabled (Infinite)"));
  } else {
    debugPrint(count);
    debugPrintln(F(" times"));
  }
}

// ----------------------------------- Frame / Handshake Settings -----------------------------------
// When no preset is applied (the Custom protocol) the user sets these by hand.

void KLine_Protocol::setHeader(const uint8_t* header, uint8_t length) {
  _headerLength = (length > sizeof(_header)) ? sizeof(_header) : length;
  if (header != nullptr && _headerLength > 0) {
    memcpy(_header, header, _headerLength);
  }

  debugPrint(F("✅ Header set to: "));
  for (uint8_t i = 0; i < _headerLength; i++) {
    debugPrintHex(_header[i]);
    debugPrint(F(" "));
  }
  debugPrintln(F(""));
}

void KLine_Protocol::setLengthMode(OBD2LengthMode mode) {
  _lengthMode = mode;
}

// The init method is chosen independently of the protocol. Because
// setProtocol() restores the protocol default, this call must ALWAYS come
// after setProtocol().
// Writing the same value again prints no log. During the connect sequence this
// function is called twice - first applyProtocolPresets() loads the protocol
// DEFAULT init, then _tryProtocol() applies the init REQUESTED for that round.
// If the two are the same (ISO9141 + 5 baud, say) there is no point in seeing
// the same line twice; if they differ, the second line really does tell you it
// was overwritten.
void KLine_Protocol::setInitType(OBD2InitType type) {
  if (_initType == type) return;

  _initType = type;
  debugPrint(F("✅ Init Type set to: "));
  debugPrintln(getInitTypeName(_initType));
}

void KLine_Protocol::setInitType(uint8_t typeId) {
  setInitType((OBD2InitType)typeId);
}

OBD2InitType KLine_Protocol::getInitType() {
  return _initType;
}

const char* KLine_Protocol::getInitTypeName(OBD2InitType type) {
  switch (type) {
    case Init_5Baud: return "5 Baud Init";
    case Init_Fast: return "Fast Init";
    case Init_Ping: return "Ping";
    default: return "None";
  }
}

void KLine_Protocol::setInitAddress(uint8_t address) {
  defaultInitAddress = address;
  debugPrint(F("✅ New Init Address set to: "));
  debugPrintHex(address);
  debugPrintln(F(""));
}

// Whether the parity bit of the 5 baud init data is calculated as even or odd.
// send5baud() uses this setting during the handshake.
void KLine_Protocol::setInitParity(OBD2Parity parity) {
  _isOddParity = (parity == Parity_Odd);
  debugPrint(F("✅ Init Parity set to: "));
  debugPrintln(_isOddParity ? "Odd" : "Even");
}

void KLine_Protocol::setWakeUpDelay(uint16_t delayMs) {
  _wakeUpDelayMs = delayMs;
  debugPrint(F("✅ WakeUp Delay set to: "));
  debugPrint(delayMs);
  debugPrintln(F(" ms"));
}

// ----------------------------------- Frame Building -----------------------------------

// The packet is built according to the settings of the currently selected protocol.
uint8_t KLine_Protocol::buildFrame(const uint8_t* data, uint8_t dataLength, uint8_t* frame) {
  uint8_t actualLengthByteCount = (_lengthMode == Length_SeparateByte) ? 1 : 0;
  uint8_t checksumLength = (checksumType == Checksum_None) ? 0 : 1;
  uint8_t fullDataLength = _headerLength + actualLengthByteCount + dataLength + checksumLength;

  // 1. Copy Header
  if (_headerLength > 0) {
    memcpy(frame, _header, _headerLength);
  }

  // 2. Handle Length Mapping
  if (_lengthMode == Length_InHeader && _headerLength > 0) {
    // Standard ISO 14230 (KWP2000) length in first byte (usually bits 5-0)
    frame[0] = (frame[0] & ~_lengthMask) | (dataLength & _lengthMask);
  } else if (_lengthMode == Length_SeparateByte) {
    if (_lengthIncludesFrame) {
      // DS2 length includes EVERYTHING: Header(1) + Len(1) + Payload + Checksum(1)
      frame[_headerLength] = fullDataLength;
    } else {
      frame[_headerLength] = dataLength;
    }
  }

  // 3. Copy Payload
  uint8_t dataStartOffset = _headerLength + actualLengthByteCount;
  memcpy(&frame[dataStartOffset], data, dataLength);

  // 4. Compute Checksum
  if (checksumType != Checksum_None) {
    frame[fullDataLength - 1] = calculateChecksum(frame, fullDataLength - 1, checksumType);
  }

  return fullDataLength;
}

void KLine_Protocol::writeData(const uint8_t* data, uint8_t dataLength) {
  uint8_t frame[dataLength + MAX_FRAME_OVERHEAD];
  uint8_t frameLength = buildFrame(data, dataLength, frame);
  sendBytes(frame, frameLength);
}

// ----------------------------------- Block Framing (KW1281) -----------------------------------
// Instead of a normal packet with a header and a checksum, these protocols
// build a block with a length byte and a message counter, and every byte is
// acknowledged by the other side sending its complement back.
// Putting the bytes on the bus is the job of the core (writeByte / readByte);
// assembling the block is the job of this layer.

void KLine_Protocol::writeBlock(const uint8_t* dataArray, uint8_t length) {
  debugPrintln(F("➡️ Sending Full Data..."));
  uint8_t newLength = length + 2;
  uint8_t newArray[newLength];

  blockMessageCount = blockMessageCount + 1;

  newArray[0] = length + 1;
  newArray[1] = blockMessageCount;
  for (uint8_t i = 0; i < length; i++) {
    newArray[i + 2] = dataArray[i];
  }

  for (size_t i = 0; i < newLength; i++) {
    writeByte(newArray[i]);

    // The LAST byte of a block (the $03 end marker) is never acknowledged by
    // the receiver. Waiting for a complement that will never arrive costs a
    // full P2 timeout on every single block, so it is skipped for that byte.
    if (i == newLength - 1) break;
    if (readByte() == -1) break;
  }

  debugPrint(F("✅ Message Count: "));
  debugPrintHex(blockMessageCount);
  debugPrint(F("   Sent All Data: "));
  for (int i = 0; i < newLength; i++) {
    debugPrintHex(newArray[i]);
    debugPrint(F(" "));
  }
  debugPrintln(F(""));
}

uint8_t KLine_Protocol::readBlock() {
  debugPrintln(F("Reading Full Data..."));
  uint8_t receiveLength = 0;
  uint8_t messageLength = 0;

  memset(resultBuffer, 0, sizeof(resultBuffer));
  while (true) {
    int receivedByte = readByte();
    if (receivedByte > -1) {
      if (receiveLength < sizeof(resultBuffer)) {
        resultBuffer[receiveLength] = receivedByte;
      }
      if (receiveLength == 0) messageLength = receivedByte;
      if (receiveLength == 1) blockMessageCount = receivedByte;

      receiveLength++;

      if (messageLength < receiveLength) {
        debugPrint(F("✅ Message Count: "));
        debugPrintHex(blockMessageCount);
        debugPrint(F("   Length: "));
        debugPrintHex(messageLength);
        debugPrint(F("   Received All Data: "));
        for (int i = 0; i < receiveLength; i++) {
          debugPrintHex(resultBuffer[i]);
          debugPrint(F(" "));
        }
        debugPrintln(F(""));
        _lastReadLength = receiveLength;
        return receiveLength;
      }

      writeByte(~receivedByte);
    } else {
      break;
    }
  }
  _lastReadLength = 0;
  return 0;
}

// ----------------------------------- Stream Framing (KW82) -----------------------------------
// KW82 does not do request / response. Once the handshake is over the ECU
// starts repeating one answer forever; sending a request only changes WHICH
// answer it repeats. The repeats follow each other with little or no gap, so
// the gap based framing of readData() can easily start mid packet.
//
// The frame is self delimiting, which makes a length driven read reliable:
//
//   [length N] [payload ...] [marker] [checksum]        total = N + 2
//
// The checksum is the Modulo-256 sum of the FIRST N bytes - the marker sitting
// just before it is NOT part of it. Verified on two real packets from an Opel
// instrument cluster:
//   ID    : len $20, marker $06, checksum $4F = sum(byte[0..31])
//   Live  : len $21, marker $05, checksum $F3 = sum(byte[0..32])
// (The marker is $00 in tester requests and non-zero in ECU answers; what it
// counts is not known yet.)
//
// Because the generic verifyChecksum() sums everything except the last byte it
// would include the marker and always fail here, which is why the checksum is
// checked in this function instead.
//
// Starting mid stream is expected, so the frame is found with a SLIDING WINDOW
// rather than by guessing: every arriving byte is asked "does a complete,
// checksum-correct frame end on you?". Consuming length+2 bytes on a wrong
// guess instead would stall on the long runs of $00 inside these packets and
// never line up.
//
// Once a frame is found the read stops exactly at its last byte, so the next
// call starts on a boundary and reads a single packet. Verified against the
// real repeating stream: it locks on from every one of the 35 possible start
// offsets, consumes at most 69 bytes doing so, and stays aligned afterwards.
uint8_t KLine_Protocol::readPacket() {
  uint16_t filled = 0;

  while (filled < sizeof(resultBuffer)) {
    const int received = readByte();
    if (received < 0) break;  // nothing more on the bus

    resultBuffer[filled++] = (uint8_t)received;
    const uint16_t end = filled - 1;

    // Does a complete frame END on the byte that just arrived? A start only
    // qualifies if its own length byte points exactly at this position, so the
    // test is cheap and a wrong boundary is rejected by the checksum.
    for (uint16_t start = 0; start < end; start++) {
      const uint8_t declared = resultBuffer[start];
      if (declared < 1) continue;
      if (start + (uint16_t)declared + 1 != end) continue;
      if (checksum8_Modulo256(&resultBuffer[start], declared) != resultBuffer[end]) continue;

      const uint8_t total = declared + 2;
      if (start > 0) {
        debugPrint(F("↩️ Stream resynced, skipped "));
        debugPrint(start);
        debugPrintln(F(" byte(s)"));
        memmove(resultBuffer, &resultBuffer[start], total);
      }

      debugPrint(F("✅ Stream packet: len "));
      debugPrintHex(declared);
      debugPrint(F("   marker "));
      debugPrintHex(resultBuffer[total - 2]);
      debugPrint(F("   Received: "));
      for (uint8_t i = 0; i < total; i++) {
        debugPrintHex(resultBuffer[i]);
        debugPrint(F(" "));
      }
      debugPrintln(F(""));

      _lastReadLength = total;
      return total;
    }
  }

  debugPrintln(F("❌ No valid stream packet found."));
  _lastReadLength = 0;
  return 0;
}

// ----------------------------------- Connection Sequence -----------------------------------

bool KLine_Protocol::connect() {
  if (connectionStatus) return true;

  debugPrintln(F("🔍 Starting Connection Sequence..."));

  // If a protocol was chosen by hand nothing is tried: its settings were
  // already loaded by setProtocol(), so its init method is run directly.
  if (selectedProtocol != Automatic) return _tryProtocol(selectedProtocol, _initType);

  debugPrintln(F("🛠️ Automatic mode: Testing standard protocols..."));

  // 1. Fast init - only ISO14230 speaks it, so the protocol is known up front.
  if (_tryProtocol(ISO14230, Init_Fast)) {
    debugPrint(F("🎉 SUCCESS! Auto-detected: "));
    debugPrintln(getProtocolName(connectedProtocol));
    return true;
  }

  // 2. Slow init - NOT run on behalf of a protocol, but to find the protocol.
  if (_trySlowInitAuto()) {
    debugPrint(F("🎉 SUCCESS! Auto-detected: "));
    debugPrintln(getProtocolName(connectedProtocol));
    return true;
  }

  debugPrintln(F("❌ No Protocol Matched. Initialization Failed."));
  return false;
}

// ---------------------------------------------------------------------------
//   The 5 baud step of Automatic mode
// ---------------------------------------------------------------------------
// The idea here: the 5 baud HANDSHAKE of ISO9141 and ISO14230 is identical
// (10400 baud, 8N1, address $33, even parity, same W timings). The difference
// is in the shape of the packet - header, length byte, checksum - which only
// starts AFTER the handshake has finished. So there is no point in picking a
// protocol first and loading its packet settings: you would load them and then
// have to switch to the other one based on the keywords anyway.
//
// That is why the order is reversed here:
//   1. only the handshake settings are loaded (without a protocol)
//   2. $33 is sent, the ECU returns its keywords
//   3. the keywords name the protocol
//   4. the packet settings are loaded from the NOW KNOWN protocol
//
// Side benefit: the log no longer shows a "trying ISO9141" round that does not
// really exist - a single 5 baud round covers both ISO9141 and ISO14230 slow init.
bool KLine_Protocol::_trySlowInitAuto() {
  debugPrintln(F("\n--- Trial Round: 5 Baud Init (protocol comes from the keywords) ---"));

  currentProtocol = None;
  connectedProtocol = None;

  // 1. Only the settings the handshake needs
  applyHandshakeSettings(getProtocolConfig(SLOW_INIT_HANDSHAKE_SOURCE));
  setInitType(Init_5Baud);

  // 2-3. Do the handshake; trySlowInit() detects the protocol from the keywords
//      and writes it into connectedProtocol.
  if (!trySlowInit()) return false;
  if (connectedProtocol == None) return false;

  // 4. Packet settings from the now known protocol. applyFrameSettings() does
  //    not touch the init method, so Init_5Baud stays as it is.
  debugPrint(F("✅ Applying frame settings for: "));
  debugPrintln(getProtocolName(connectedProtocol));
  applyFrameSettings(getProtocolConfig(connectedProtocol));

  currentProtocol = connectedProtocol;
  connectionStatus = true;
  return true;
}

bool KLine_Protocol::_tryProtocol(OBD2Protocol p, OBD2InitType init) {
  // "None" = no protocol selected. Not to be confused with Init_None (no
  // handshake, but there is a protocol) - here there is nothing to try.
  if (p == None) {
    debugPrintln(F("❌ No protocol selected."));
    return false;
  }

  debugPrint(F("\n--- Trial Round: "));
  debugPrint(getProtocolName(p));
  debugPrintln(F(" ---"));

  const OBD2ProtocolConfig* config = getProtocolConfig(p);

  // 1. Prepare Settings
  currentProtocol = p;
  if (selectedProtocol == Automatic) {
    applyProtocolPresets(p);  // loads the defaults (including init)
    setInitType(init);        // apply the init method requested for this round
  }

  // Address overrides for specific protocols if they haven't been changed from default (0x33)
  uint8_t originalAddr = defaultInitAddress;
  if (config != nullptr && config->initAddress != 0x33 && originalAddr == 0x33) {
    setInitAddress(config->initAddress);
  }

  bool success = false;

  // 2. Perform Handshake based on the ACTIVE init type.
  //    This is the default from the protocol table; setInitType() may have
  //    overridden it. Protocols without a config (Custom) pass through here too -
  //    a user who sets everything by hand can connect with Init_None, without a
  //    handshake.
  debugPrint(F("🔧 Init Type: "));
  debugPrintln(getInitTypeName(_initType));

  switch (_initType) {
    case Init_5Baud:
      success = trySlowInit();
      break;
    case Init_Fast:
      success = tryFastInit();
      break;
    case Init_Ping: {
      const uint8_t pingData[] = {0x00}; // DS2 simple ping
      writeData(pingData, 1);
      if (readData()) success = true;
      break;
    }
    case Init_None:
      // No handshake: the bus is assumed ready to use.
      debugPrintln(F("ℹ️ No handshake required - assuming line is ready."));
      success = true;
      break;
  }

  // Restore address if we overrode it but it failed
  if (!success) {
    defaultInitAddress = originalAddr;
    return false;
  }

  // trySlowInit() looks at the keywords sent by the ECU, detects whether it is
  // ISO9141 or ISO14230 and writes that into connectedProtocol. This detection
  // is meaningful ONLY for those two families: KW1281 / KW82 also use 5 baud
  // init, but the keyword distinction does not apply to them - whichever
  // protocol was selected stays.
  bool familyDetected = (p == ISO9141 || p == ISO14230) &&
                        connectedProtocol != None && connectedProtocol != p;
  if (familyDetected) {
    applyProtocolPresets(connectedProtocol);
    setInitType(init);  // presets reset init to the default, put the selected one back
  } else {
    connectedProtocol = p;
  }
  currentProtocol = connectedProtocol;
  connectionStatus = true;

  return true;
}

// ----------------------------------- Handshakes -----------------------------------

bool KLine_Protocol::trySlowInit() {
  // currentProtocol == None -> the protocol-less round of Automatic mode
  // (_trySlowInitAuto). Since it is not yet known which protocol this is,
  // printing this instead of a name is the honest thing to do: saying "trying
  // ISO9141" was misleading while both were really being tried at once.
  debugPrint(F("🔁 Trying Handshake for: "));
  debugPrintln((currentProtocol == None) ? "ISO9141 / ISO14230 (5 baud)"
                                         : getProtocolName(currentProtocol));

  toggleSerial(false);
  delay(_wakeUpDelayMs);
  send5baud(defaultInitAddress, _isOddParity); // Uses the parity from the preset
  toggleSerial(true);

  uint16_t oldP1 = _p1Time;
  bool oldChecksumVerify = _verifyChecksum;
  setP1Time(_w2Time);
  setChecksumVerify(false);

  if (readData() < 3 || resultBuffer[0] != 0x55) {
    setP1Time(oldP1);
    setChecksumVerify(oldChecksumVerify);
    return false;
  }
  // Equal keywords mean ISO9141 is answering, different ones mean ISO14230 (KWP2000)
  OBD2Protocol detectedProtocol = (resultBuffer[1] == resultBuffer[2]) ? ISO9141 : ISO14230;

  debugPrint(F("🔎 Keywords: "));
  debugPrintHex(resultBuffer[1]);
  debugPrint(F(" "));
  debugPrintHex(resultBuffer[2]);
  debugPrint(F(" -> "));
  debugPrintln(getProtocolName(detectedProtocol));

  // Send inverted KW2 to acknowledge
  debugPrint(F("➡️ Sending Inverted KW2: ")); debugPrintHex(~resultBuffer[2]); debugPrintln(F(""));
  _serial->write(~resultBuffer[2]);
  clearEcho(1);
  setP1Time(oldP1);

  // KW82 does not answer the inverted keyword with a single byte either: it
  // starts streaming its identification packet straight away and repeats it
  // forever. Reading one COMPLETE packet is what proves the ECU is there.
  if (currentProtocol == KW82) {
    debugPrintln(F("✅ Stream protocol handshake done - reading first packet."));
    setChecksumVerify(oldChecksumVerify);
    if (!readPacket()) return false;
    connectedProtocol = currentProtocol;
    connectionStatus = true;
    return true;
  }

  if (currentProtocol == KW1281) {
    debugPrintln(F("✅ Block protocol handshake done - ECU will now send blocks."));
    setChecksumVerify(oldChecksumVerify);
    blockMessageCount = 0;
    connectedProtocol = currentProtocol;
    connectionStatus = true;
    return true;
  }

  // Even if we fail here we MUST turn verification back on: it was switched off
  // for the handshake, and if it is left off every packet from now on would be
  // accepted without passing a checksum check.
  if (!readData()) {
    setChecksumVerify(oldChecksumVerify);
    return false;
  }
  setChecksumVerify(oldChecksumVerify);

  connectedProtocol = detectedProtocol;
  connectionStatus = true;
  return true;
}

bool KLine_Protocol::tryFastInit() {
  debugPrint(F("🔁 Trying Fast Init for: "));
  debugPrintln(getProtocolName(currentProtocol));

  toggleSerial(false);
  delay(_wakeUpDelayMs);

  sendFastInitPulse();

  toggleSerial(true);

  // Resulting packet: 81 <Address> F1 81 <CS>
  writeData((uint8_t[]){0x81});

  if (!readData()) return false;

  if (resultBuffer[3] == 0xC1) {
    connectionStatus = true;
    return true;
  }

  return false;
}
