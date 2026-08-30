/*
 * OBD2_KLine Library - MukiTech
 * -----------------------------
 * Standard OBD2 (SAE J1979) diagnostic services implementation.
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

#include "OBD2_Standard.h"

// ----------------------------------- Live Data -----------------------------------

float OBD2_Standard::getLiveData(uint8_t pid) {
  return getPID(read_LiveData, pid);
}

float OBD2_Standard::getFreezeFrame(uint8_t pid) {
  return getPID(read_FreezeFrame, pid);
}

float OBD2_Standard::getPID(uint8_t mode, uint8_t pid) {
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
      return A;
    case 0x04:                                      // Engine Load (%)
      return A * 100.0f / 255.0f;
    case 0x05:                                      // Coolant Temperature (°C)
      return A - 40.0f;
    case 0x06:                                      // Short Term Fuel Trim Bank 1 (%)
    case 0x07:                                      // Long Term Fuel Trim Bank 1 (%)
    case 0x08:                                      // Short Term Fuel Trim Bank 2 (%)
    case 0x09:                                      // Long Term Fuel Trim Bank 2 (%)
      return A * 100.0f / 128.0f - 100.0f;
    case 0x0A:                                      // Fuel Pressure (kPa)
      return A * 3.0f;
    case 0x0B:                                      // Intake Manifold Absolute Pressure (kPa)
      return A;
    case 0x0C:                                      // RPM
      return ((A * 256.0f) + B) / 4.0f;
    case 0x0D:                                      // Speed (km/h)
      return A;
    case 0x0E:                                      // Timing Advance (°)
      return A / 2.0f - 64.0f;
    case 0x0F:                                      // Intake Air Temperature (°C)
      return A - 40.0f;
    case 0x10:                                      // MAF Flow Rate (grams/sec)
      return ((A * 256.0f) + B) / 100.0f;
    case 0x11:                                      // Throttle Position (%)
      return A * 100.0f / 255.0f;
    case 0x12:                                      // Commanded Secondary Air Status (bit encoded)
    case 0x13:                                      // Oxygen Sensors Present 2 Banks (bit encoded)
      return A;
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
      return A;
    case 0x1F:                                      // Run Time Since Engine Start (seconds)
    case 0x21:                                      // Distance Traveled With MIL On (km)
      return (A * 256.0f) + B;
    case 0x22:                                      // Fuel Rail Pressure (kPa)
      return ((A * 256.0f) + B) * 0.079f;
    case 0x23:                                      // Fuel Rail Gauge Pressure (kPa)
      return ((A * 256.0f) + B) / 10.0f;
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
      return A * 100.0f / 255.0f;
    case 0x2D:                                      // EGR Error (%)
      return A * 100.0f / 128.0f - 100.0f;
    case 0x2E:                                      // Commanded Evaporative Purge (%)
    case 0x2F:                                      // Fuel Tank Level Input (%)
      return A * 100.0f / 255.0f;
    case 0x30:                                      // Warm-ups Since Codes Cleared (count)
      return A;
    case 0x31:                                      // Distance Traveled Since Codes Cleared (km)
      return (A * 256.0f) + B;
    case 0x32:                                      // Evap System Vapor Pressure (Pa)
      return ((A * 256.0f) + B) / 4.0f;
    case 0x33:                                      // Absolute Barometric Pressure (kPa)
      return A;
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
      return ((A * 256.0f) + B) / 10.0f - 40.0f;
    case 0x41:                                      // Monitor status this drive cycle (bit encoded)
      return A;
    case 0x42:                                      // Control module voltage (V)
      return ((A * 256.0f) + B) / 1000.0f;
    case 0x43:                                      // Absolute load value (%)
      return ((A * 256.0f) + B) * 100.0f / 255.0f;
    case 0x44:                                      // Fuel/Air commanded equivalence ratio (lambda)
      return ((A * 256.0f) + B) / 32768.0f;         // ratio
    case 0x45:                                      // Relative throttle position (%)
      return A * 100.0f / 255.0f;
    case 0x46:                                      // Ambient air temp (°C)
      return A - 40.0f;
    case 0x47:                                      // Absolute throttle position B (%)
    case 0x48:                                      // Absolute throttle position C (%)
    case 0x49:                                      // Accelerator pedal position D (%)
    case 0x4A:                                      // Accelerator pedal position E (%)
    case 0x4B:                                      // Accelerator pedal position F (%)
    case 0x4C:                                      // Commanded throttle actuator (%)
      return A * 100.0f / 255.0f;
    case 0x4D:                                      // Time run with MIL on (min)
    case 0x4E:                                      // Time since trouble codes cleared (min)
      return (A * 256.0f) + B;
    case 0x4F:                                      // Max values for sensors (ratio, V, mA, kPa)
    case 0x50:                                      // Maximum value for air flow rate from mass air flow sensor (g/s)
    case 0x51:                                      // Fuel Type (bit encoded)
      return A;
    case 0x52:                                      // Ethanol fuel (%)
      return A * 100.0f / 255.0f;
    case 0x53:                                      // Absolute evap system pressure (kPa)
      return ((A * 256.0f) + B) / 200.0f;
    case 0x54:                                      // Evap system vapor pressure (Pa)
      return (A * 256.0f) + B;
    case 0x55:                                      // Short term secondary oxygen sensor trim, A: bank 1, B: bank 3 (%)
    case 0x56:                                      // Long term primary oxygen sensor trim, A: bank 1, B: bank 3 (%)
    case 0x57:                                      // Short term secondary oxygen sensor trim, A: bank 2, B: bank 4 (%)
    case 0x58:                                      // Long term secondary oxygen sensor trim, A: bank 2, B: bank 4 (%)
      return A * 100.0f / 128.0f - 100.0f;
    case 0x59:                                      // Fuel rail absolute pressure (kPa)
      return ((A * 256.0f) + B) * 10.0f;
    case 0x5A:                                      // Relative accelerator pedal position (%)
    case 0x5B:                                      // Hybrid battery pack remaining life (%)
      return A * 100.0f / 255.0f;
    case 0x5C:                                      // Engine oil temperature (°C)
      return A - 40.0f;
    case 0x5D:                                      // Fuel injection timing (°)
      return ((A * 256.0f) + B) / 128.0f - 210.0f;
    case 0x5E:                                      // Engine fuel rate (L/h)
      return ((A * 256.0f) + B) / 20.0f;
    case 0x5F:                                      // Emission requirements to which vehicle is designed (bit encoded)
      return A;
    case 0x61:                                      // Driver's demand engine - percent torque (%)
    case 0x62:                                      // Actual engine - percent torque (%)
      return A - 125.0f;
    case 0x63:                                      // Engine reference torque (Nm)
      return (A * 256.0f) + B;
    default:
      return -4;                                    // Unknown PID
  }
}

// ----------------------------------- DTCs -----------------------------------

uint8_t OBD2_Standard::readStoredDTCs() {
  return readDTCs(0x03);
}

uint8_t OBD2_Standard::readPendingDTCs() {
  return readDTCs(0x07);
}

uint8_t OBD2_Standard::readDTCs(uint8_t mode) {
  // Request: C2 33 F1 03 F3
  // example Response: 87 F1 11 43 01 70 01 34 00 00 72
  // example Response: 87 F1 11 43 00 00 CC
  int dtcCount = 0;
  String* targetArray = nullptr;

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

String OBD2_Standard::getStoredDTC(uint8_t index) {
  if (index >= 0) return storedDTCBuffer[index];
  return "";
}

String OBD2_Standard::getPendingDTC(uint8_t index) {
  if (index >= 0) return pendingDTCBuffer[index];
  return "";
}

bool OBD2_Standard::clearDTCs() {
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

String OBD2_Standard::getVehicleInfo(uint8_t pid) {
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

  for (int j = 0; j < messageCount; j++) {
    if (!readData()) break;
    if (pid == 0x02 && j == 0) {
      dataArray[arrayNum++] = resultBuffer[9];
      continue;
    }

    for (int i = 0; i < 4; i++) {
      if (arrayNum < sizeof(dataArray)) {
        dataArray[arrayNum++] = resultBuffer[i + 6];
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

uint8_t OBD2_Standard::readSupportedLiveData() {
  return readSupportedData(read_LiveData);
}

uint8_t OBD2_Standard::readSupportedFreezeFrame() {
  return readSupportedData(read_FreezeFrame);
}

uint8_t OBD2_Standard::readSupportedOxygenSensors() {
  return readSupportedData(test_OxygenSensors);
}

uint8_t OBD2_Standard::readSupportedOtherComponents() {
  return readSupportedData(test_OtherComponents);
}

uint8_t OBD2_Standard::readSupportedOnBoardComponents() {
  return readSupportedData(control_OnBoardComponents);
}

uint8_t OBD2_Standard::readSupportedVehicleInfo() {
  return readSupportedData(read_VehicleInfo);
}

uint8_t OBD2_Standard::readSupportedData(uint8_t mode) {
  int supportedCount = 0;
  int pidIndex = 0;
  int startByte = 0;
  int arraySize = 32;  // Size of supported data arrays
  uint8_t* targetArray = nullptr;

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

    if(mode == read_FreezeFrame ) writeData((uint8_t[]){mode, pidCmds[n], 0x00});
    else writeData((uint8_t[]){mode, pidCmds[n]});

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

uint8_t OBD2_Standard::getSupportedData(uint8_t mode, uint8_t index) {
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
