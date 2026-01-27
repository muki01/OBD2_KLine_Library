#include "OBD2_KLine.h"  // Include the library for OBD2 K-Line communication

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
#include <AltSoftSerial.h>
AltSoftSerial Alt_Serial;
OBD2_KLine KLine(Alt_Serial, 10400, 8, 9);  // Uses AltSoftSerial at 10400 baud, with RX on pin 8 and TX on pin 9.
#elif defined(ESP32)
OBD2_KLine KLine(Serial1, 10400, 10, 11);  // Uses Hardware Serial (Serial1) at 10400 baud, with RX on pin 10 and TX on pin 11.
#else
#error "Unsupported board! This library currently supports Arduino Uno, Nano, Mega, and ESP32. Please select a compatible board in your IDE."
#endif

void setup() {
  Serial.begin(115200);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 K-Line Read DTCs Example");

  KLine.setDebug(Serial);          // Optional: outputs debug messages to the selected serial port
  KLine.setProtocol("Automatic");  // Optional: communication protocol (default: Automatic; supported: ISO9141, ISO14230_Slow, ISO14230_Fast, Automatic)
  KLine.setByteWriteInterval(5);   // Optional: delay (ms) between bytes when writing
  KLine.setInterByteTimeout(60);   // Optional: sets the maximum inter-byte timeout (ms) while receiving data
  KLine.setReadTimeout(1000);      // Optional: maximum time (ms) to wait for a response after sending a request

  KLine.setInitAddress(0x33);                 // Optional: Sets the target ECU address used during the 5-baud Slow Init sequence.
  KLine.setISO9141Header(0x68, 0x6A, 0xF1);   // Optional: Configures the 3-byte header (Priority, Receiver, Transmitter) for ISO9141.
  KLine.setISO14230Header(0xC0, 0x33, 0xF1);  // Optional: Configures the 3-byte header (Format, Receiver, Transmitter) for KWP2000.
  KLine.setLengthMode(true);                  // Optional: Defines if data length is embedded in the header or sent as a separate byte.
  KLine.setChecksumType(2);                   // Optional: Selects checksum method (0: None, 1: XOR, 2: Modulo 256, 3: Two's Complement).

  Serial.println("OBD2 Starting.");
}

void loop() {
  // Attempt to initialize OBD2 communication
  if (KLine.initOBD2()) {
    int storedDTCLength = KLine.readStoredDTCs();  // Read stored Diagnostic Trouble Codes (DTCs)
    // int storedDTCLength = KLine.readDTCs(0x03);
    if (storedDTCLength > 0) {
      Serial.println("Stored DTCs:");
      for (int i = 0; i < storedDTCLength; i++) {
        String dtc = KLine.getStoredDTC(i);  // Get stored DTC
        Serial.println(dtc);                 // Print the stored DTC
      }
      Serial.println();
    } else {
      Serial.println("No Stored DTCs");
    }

    int pendingDTCLength = KLine.readPendingDTCs();  // Read pending Diagnostic Trouble Codes (DTCs)
    // int pendingDTCLength = KLine.readDTCs(0x07);
    if (pendingDTCLength > 0) {
      Serial.println("Pending DTCs:");
      for (int i = 0; i < pendingDTCLength; i++) {
        String dtc = KLine.getPendingDTC(i);  // Get pending DTC
        Serial.println(dtc);                  // Print the pending DTC
      }
      Serial.println();
    } else {
      Serial.println("No Pending DTCs");
    }
  }
}
