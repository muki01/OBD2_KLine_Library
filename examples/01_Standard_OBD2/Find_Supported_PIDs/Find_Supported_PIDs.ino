/*
 * OBD2 K-Line - Find Supported PIDs
 *
 * Queries and lists all PIDs supported by the vehicle ECU across
 * standard OBD2 service modes. Useful for discovering what data
 * the ECU can provide before implementing a diagnostic application.
 *
 * Scans : Mode 01 (Live Data)      | Mode 02 (Freeze Frame)
 *         Mode 05 (Oxygen Sensors) | Mode 06 (Other Components)
 *         Mode 08 (On-Board Ctrl)  | Mode 09 (Vehicle Info)
 *
 * Protocol  : Standard OBD2 (ISO9141, ISO14230)
 * Baud Rate : 10400 baud
 * Checksum  : Modulo-256
 * Tested On : Generic OBD2-compliant vehicles
 *
 * Reference : For a full list of OBD2 PIDs, visit:
 *             https://en.wikipedia.org/wiki/OBD-II_PIDs
 */

#include "OBD2_KLine.h"          // core: connection + protocol layer
#include "ecus/OBD2_Standard.h"  // standard OBD2 diagnostics -> OBD2_KLine

OBD2_KLine KLine;

// #include <AltSoftSerial.h>
// AltSoftSerial altSerial;

// #include <SoftwareSerial.h>
// SoftwareSerial softSerial(10, 11);

#define OBD_SERIAL        Serial1
#define OBD_DEBUG_SERIAL  Serial
#define OBD_RX_PIN        10
#define OBD_TX_PIN        11

// ── Helper: print supported PID list for a given mode ────────────────────────
void printSupportedPIDs(const char* label, uint8_t mode, int count) {
  Serial.print(F("[MODE 0")); Serial.print(mode, HEX); Serial.print(F("] "));
  Serial.print(label); Serial.print(F(": "));
  if (count > 0) {
    for (int i = 0; i < count; i++) {
      byte pid = KLine.getSupportedData(mode, i);
      if (pid < 0x10) Serial.print('0');
      Serial.print(pid, HEX);
      Serial.print(' ');
    }
    Serial.println();
  } else {
    Serial.println(F("Not supported"));
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  KLine.setSerial(OBD_SERIAL);
  KLine.setPins(OBD_RX_PIN, OBD_TX_PIN);

  // ── Optional Settings ──
  KLine.setDebug(OBD_DEBUG_SERIAL);        // Enable debug output to Serial
  KLine.setProtocol(Automatic);  // Automatic, ISO9141, ISO14230, KW1281, DS2, KW82, Custom
  // The init method is a separate axis; call it AFTER setProtocol() to override
  // the protocol default:  KLine.setInitType(Init_5Baud);  // or Init_Fast


  Serial.println(F("=== OBD2 K-Line | Supported PID Scanner ==="));
  Serial.println(F("Scanning Modes: 01, 02, 05, 06, 08, 09"));
  Serial.println(F("==========================================="));
}

void loop() {
  if (!KLine.isConnected()) {
    Serial.println(F("Connecting to ECU..."));
    if (!KLine.connect()) {
      Serial.println(F("Connection failed. Retrying..."));
      return;
    }
    Serial.println(F("Connection established."));
    Serial.println(F("----------------------------------------"));
  }

  printSupportedPIDs("Live Data",         0x01, KLine.readSupportedLiveData());          delay(500);
  printSupportedPIDs("Freeze Frame",      0x02, KLine.readSupportedFreezeFrame());       delay(500);
  printSupportedPIDs("Oxygen Sensors",    0x05, KLine.readSupportedOxygenSensors());     delay(500);
  printSupportedPIDs("Other Components",  0x06, KLine.readSupportedOtherComponents());   delay(500);
  printSupportedPIDs("On-Board Ctrl",     0x08, KLine.readSupportedOnBoardComponents()); delay(500);
  printSupportedPIDs("Vehicle Info",      0x09, KLine.readSupportedVehicleInfo());

  Serial.println(F("----------------------------------------"));
  delay(10000);
}
