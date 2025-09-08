#include "OBD2_KLine.h"  // Include the library for OBD2 K-Line communication
// #include <AltSoftSerial.h>  // Optional alternative software serial (not used here)
// AltSoftSerial Alt_Serial;   // Create an alternative serial object (commented out)

// ---------------- Create an OBD2_KLine object for communication.
OBD2_KLine KLine(Serial1, 10400, 10, 11);  // Uses Hardware Serial (Serial1) at 10400 baud, with RX on pin 10 and TX on pin 11.
// OBD2_KLine KLine(Alt_Serial, 10400, 8, 9); // Uses AltSoftSerial at 10400 baud, with RX on pin 8 and TX on pin 9.

void setup() {
  Serial.begin(115200);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 K-Line Read DTCs Example");

  KLine.setDebug(Serial);          // Optional: outputs debug messages to the selected serial port
  KLine.setProtocol("Automatic");  // Optional: communication protocol (default: Automatic; supported: ISO9141, ISO14230_Slow, ISO14230_Fast, Automatic)
  KLine.setByteWriteInterval(5);   // Optional: delay (ms) between bytes when writing
  KLine.setInterByteTimeout(60);   // Optional: sets the maximum inter-byte timeout (ms) while receiving data
  KLine.setReadTimeout(1000);      // Optional: maximum time (ms) to wait for a response after sending a request

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
