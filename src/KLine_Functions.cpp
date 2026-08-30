/*
 * OBD2_KLine Library - MukiTech
 * -----------------------------
 * Shared helper functions implementation.
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

#include "KLine_Functions.h"

// ----------------------------------- Checksum -----------------------------------

const char* getChecksumName(OBD2Checksum checksum) {
  switch (checksum) {
    case Checksum_XOR: return "XOR";
    case Checksum_Modulo256: return "Modulo256";
    case Checksum_TwosComplement: return "TwosComplement";
    default: return "None";
  }
}

uint8_t calculateChecksum(const uint8_t* dataArray, uint16_t length, OBD2Checksum checksum) {
  switch (checksum) {
    case Checksum_XOR:
      return checksum8_XOR(dataArray, length);
    case Checksum_Modulo256:
      return checksum8_Modulo256(dataArray, length);
    case Checksum_TwosComplement:
      return checksum8_TwosComplement(dataArray, length);
    default:
      return 0;
  }
}

uint8_t checksum8_XOR(const uint8_t* dataArray, int length) {
  uint8_t checksum = 0;
  for (int i = 0; i < length; i++) {
    checksum ^= dataArray[i];  // XOR operation
  }
  return checksum;
}

uint8_t checksum8_Modulo256(const uint8_t* dataArray, int length) {
  unsigned int sum = 0;
  for (int i = 0; i < length; i++) {
    sum += dataArray[i];
  }
  return (byte)(sum % 256);  // or (byte)sum; because uint8_t overflow also gives a mod 256 effect.
}

uint8_t checksum8_TwosComplement(const uint8_t* dataArray, int length) {
  unsigned int sum = 0;
  for (int i = 0; i < length; i++) {
    sum += dataArray[i];
  }
  byte checksum = (byte)((0x100 - (sum & 0xFF)) & 0xFF);
  return checksum;
}

// ----------------------------------- Compare -----------------------------------

bool compareData(const uint8_t* dataArray1, uint8_t length1, const uint8_t* dataArray2, uint8_t length2) {
  if (length1 != length2) return false;
  for (uint8_t i = 0; i < length1; i++) {
    if (dataArray1[i] != dataArray2[i]) {
      return false;
    }
  }
  return true;
}

// ----------------------------------- Conversion -----------------------------------

String decodeDTC(uint8_t b1, uint8_t b2) {
  String ErrorCode = "";
  static const char type_lookup[4] = {'P', 'C', 'B', 'U'};
  static const char digit_lookup[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  ErrorCode += type_lookup[(b1 >> 6) & 0x03];
  ErrorCode += digit_lookup[(b1 >> 4) & 0x03];
  ErrorCode += digit_lookup[b1 & 0x0F];
  ErrorCode += digit_lookup[(b2 >> 4) & 0x0F];
  ErrorCode += digit_lookup[b2 & 0x0F];

  return ErrorCode;
}

bool isInArray(const uint8_t* dataArray, uint8_t length, uint8_t value) {
  for (int i = 0; i < length; i++) {
    if (dataArray[i] == value) {
      return true;
    }
  }
  return false;
}

String convertBytesToHexString(const uint8_t* dataArray, uint8_t length) {
  String hexString = "";
  for (int i = 0; i < length; i++) {
    if (dataArray[i] < 0x10) hexString += "0";  // Pad leading zero
    hexString += String(dataArray[i], HEX);
  }
  hexString.toUpperCase();
  return hexString;
}

String convertHexToAscii(const uint8_t* dataArray, uint8_t length) {
  String asciiString = "";
  for (int i = 0; i < length; i++) {
    uint8_t b = dataArray[i];
    if (b >= 0x20 && b <= 0x7E) {  // Printable ASCII range
      asciiString += (char)b;
    }
  }
  return asciiString;
}
