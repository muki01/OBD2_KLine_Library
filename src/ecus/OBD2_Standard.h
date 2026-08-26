/*
 * OBD2_KLine Library - MukiTech
 * -----------------------------
 * Standard OBD2 diagnostics (the generic 0x33 functional address).
 *
 * Everything in this file is defined by the OBD2 standard (SAE J1979):
 * live data, freeze frame, DTCs, vehicle information and supported PID
 * scanning. Car / ECU specific services belong in their own files.
 *
 * The connection / handshake logic comes from KLine_Protocol, the byte level
 * communication from KLine_Core.
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

#ifndef OBD2_STANDARD_H
#define OBD2_STANDARD_H

#include <Arduino.h>

#include "../KLine_Protocol.h"  // KLine_Functions.h comes in through this too (decodeDTC, isInArray, convert*)

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

class OBD2_Standard : public KLine_Protocol {
 public:
  // uint32_t getPIDRaw(uint8_t mode, uint8_t pid);
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

 private:
  String storedDTCBuffer[20];
  String pendingDTCBuffer[20];

  uint8_t supportedLiveData[32];
  uint8_t supportedFreezeFrame[32];
  uint8_t supportedOxygenSensor[32];
  uint8_t supportedOtherComponents[32];
  uint8_t supportedControlComponents[32];
  uint8_t supportedVehicleInfo[32];
};

// The familiar name of the library. An alias for the standard OBD2 diagnostic class.
typedef OBD2_Standard OBD2_KLine;

#endif  // OBD2_STANDARD_H
