/*
 * OBD2 K-Line - Clear DTCs
 *
 * Clears all stored and pending Diagnostic Trouble Codes (DTCs)
 * from the vehicle ECU using standard OBD2 service Mode 04.
 *
 * Protocol  : Standard OBD2 (ISO9141, ISO14230)
 * Baud Rate : 10400 baud
 * Checksum  : Modulo-256
 * Tested On : Generic OBD2-compliant vehicles
 *
 * Reference : For a full list of OBD2 PIDs, visit:
 *             https://en.wikipedia.org/wiki/OBD-II_PIDs
 *
 * WARNING: This operation permanently erases all fault history from the ECU.
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

  Serial.println(F("=== OBD2 K-Line | Clear DTCs ==="));
  Serial.println(F("Mode 04 — Erase all stored and pending fault codes."));
  Serial.println(F("WARNING: This will permanently erase ECU fault history."));
  Serial.println(F("================================"));
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

  if (KLine.clearDTCs()) {
    Serial.println(F("DTCs cleared successfully."));
  } else {
    Serial.println(F("Clear command sent. Verify with a DTC read."));
  }

  delay(5000);
}
