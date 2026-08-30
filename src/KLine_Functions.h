/*
 * OBD2_KLine Library - MukiTech
 * -----------------------------
 * Shared helper functions.
 *
 * These are pure functions: give them bytes, they give you a number, a bool or
 * a String back. They touch no serial port, no protocol state and no object -
 * so they are free functions here instead of class members. Any layer (core,
 * protocol, ECU files, the user's sketch) may call them.
 *
 * What lives here:
 *   - Checksum arithmetic (XOR, Modulo256, Two's Complement)
 *   - Byte array comparison
 *   - Byte array conversion / decoding (hex string, ASCII, DTC code)
 *
 * The POLICY (which checksum type is active, is incoming data verified, which
 * buffer is compared against what) is NOT here: that is per-connection state
 * and lives in KLine_Core / KLine_Protocol, filled in from the protocol tables
 * in KLine_Protocol.cpp.
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

#ifndef KLINE_FUNCTIONS_H
#define KLINE_FUNCTIONS_H

#include <Arduino.h>

#include "KLine_Protocol.h"  // OBD2Checksum and the other shared enums

// ----------------------------------- Checksum -----------------------------------

// Calculates the checksum for the selected type (returns 0 for an unknown / None type)
uint8_t calculateChecksum(const uint8_t* dataArray, uint16_t length, OBD2Checksum checksum);

uint8_t checksum8_XOR(const uint8_t* dataArray, int length);
uint8_t checksum8_Modulo256(const uint8_t* dataArray, int length);
uint8_t checksum8_TwosComplement(const uint8_t* dataArray, int length);

// Readable name for the debug output
const char* getChecksumName(OBD2Checksum checksum);

// ----------------------------------- Compare -----------------------------------

// Compares two byte arrays. Returns false straight away if the lengths differ,
// otherwise compares byte by byte. Where either array came from is of no
// concern here.
bool compareData(const uint8_t* dataArray1, uint8_t length1, const uint8_t* dataArray2, uint8_t length2);

// Lets the compiler count both lengths: compareData(array1, array2)
template <size_t N1, size_t N2>
bool compareData(const uint8_t (&dataArray1)[N1], const uint8_t (&dataArray2)[N2]) {
  return compareData(dataArray1, N1, dataArray2, N2);
}

// One length is known, the other side is a raw pointer (e.g. resultBuffer)
template <size_t N>
bool compareData(const uint8_t (&dataArray1)[N], const uint8_t* dataArray2, uint8_t length2) {
  return compareData(dataArray1, N, dataArray2, length2);
}

// ----------------------------------- Conversion -----------------------------------

// Turns two DTC bytes into a readable error code such as "P0123"
String decodeDTC(uint8_t b1, uint8_t b2);

// Is the value present in the array
bool isInArray(const uint8_t* dataArray, uint8_t length, uint8_t value);

// Converts a byte array to an upper case hex string (e.g. {0x0A, 0xF3} -> "0AF3")
String convertBytesToHexString(const uint8_t* dataArray, uint8_t length);

// Converts the printable ASCII characters of a byte array to a string (VIN, Calibration ID)
String convertHexToAscii(const uint8_t* dataArray, uint8_t length);

#endif  // KLINE_FUNCTIONS_H
