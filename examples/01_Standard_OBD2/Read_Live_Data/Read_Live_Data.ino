/*
 * OBD2 K-Line - Read Live Data
 *
 * Reads real-time engine sensor values from the vehicle ECU
 * using standard OBD2 service Mode 01 (Current Data).
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

// Uno / Nano have no spare hardware serial, so they fall back to AltSoftSerial
// on its fixed pins (RX 8, TX 9). Every other board uses Serial1 on the pins
// defined below.
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  #include <AltSoftSerial.h>
  AltSoftSerial altSerial;
  #define OBD_SERIAL      altSerial
#else
  #define OBD_SERIAL      Serial1
#endif
#define OBD_DEBUG_SERIAL  Serial
#define OBD_RX_PIN        5
#define OBD_TX_PIN        4

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

  Serial.println(F("=== OBD2 K-Line | Live Data Monitor ==="));
  Serial.println(F("Reading: RPM, Coolant Temp, Vehicle Speed"));
  Serial.println(F("========================================"));
}

void loop() {
  if (!KLine.isConnected()) {
    Serial.println(F("Connecting to ECU..."));
    if (!KLine.connect()) {
      Serial.println(F("Connection failed. Retrying..."));
      return;
    }
    Serial.println(F("Connection established."));
  }

  float rpm         = KLine.getLiveData(0x0C);  // PID 0x0C — Engine Speed (RPM)
  Serial.print(F("Engine Speed: ")), Serial.print(rpm), Serial.println(F(" RPM"));

  float coolantTemp = KLine.getLiveData(0x05);  // PID 0x05 — Coolant Temperature (°C)
  Serial.print(F("Coolant Temp: ")), Serial.print(coolantTemp), Serial.println(F(" °C"));

  float speed       = KLine.getLiveData(0x0D);  // PID 0x0D — Vehicle Speed (KM/H)
  Serial.print(F("Vehicle Speed: ")), Serial.print(speed), Serial.println(F(" KM/H"));
}
