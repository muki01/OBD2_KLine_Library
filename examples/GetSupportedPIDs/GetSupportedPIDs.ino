#include "OBD2_KLine.h"  // Include the library for OBD2 K-Line communication
// #include <AltSoftSerial.h>  // Optional alternative software serial (not used here)
// AltSoftSerial Alt_Serial;   // Create an alternative serial object (commented out)

// ---------------- Create an OBD2_KLine object for communication.
OBD2_KLine KLine(Serial1, 10400, 10, 11);  // Uses Hardware Serial (Serial1) at 10400 baud, with RX on pin 10 and TX on pin 11.
// OBD2_KLine KLine(Alt_Serial, 10400, 8, 9); // Uses AltSoftSerial at 10400 baud, with RX on pin 8 and TX on pin 9.

void setup() {
  Serial.begin(115200);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 K-Line Get Supported PIDs Example");

  Line.setDebug(Serial);               // Optional: enable debug output on your chosen serial port
  KLine.setProtocol("ISO14230_Fast");  // Optional: Default protocol: Automatic. All protocols: ISO9141. ISO14230_Slow, ISO14230_Fast
  KLine.setWriteDelay(5);              // Optional: delay between bytes when writing to OBD (in milliseconds)
  KLine.setDataRequestInterval(60);    // Optional: delay between data reading (in milliseconds)

  Serial.println("OBD2 Starting.");
}

void loop() {
  // Attempt to initialize OBD2 communication
  if (KLine.initOBD2()) {
    int liveDataLength = KLine.readSupportedLiveData();  // Read supported live data PIDs
    if (liveDataLength > 0) {
      Serial.print("LiveData: ");
      for (int i = 0; i < liveDataLength; i++) {
        byte supported = KLine.getSupportedData(0x01, i);  // Get supported live data PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);

    int freezeFrameLength = KLine.readSupportedFreezeFrame();  // Read supported freeze frame PIDs
    if (freezeFrameLength > 0) {
      Serial.print("FreezeFrame: ");
      for (int i = 0; i < freezeFrameLength; i++) {
        byte supported = KLine.getSupportedData(0x02, i);  // Get supported freeze frame PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);

    int componentMonitoringLength = KLine.readSupportedComponentMonitoring();  // Read supported component monitoring PIDs
    if (componentMonitoringLength > 0) {
      Serial.print("Component Monitoring: ");
      for (int i = 0; i < componentMonitoringLength; i++) {
        byte supported = KLine.getSupportedData(0x06, i);  // Get supported component monitoring PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);

    int vehicleInfoLength = KLine.readSupportedVehicleInfo();  // Read supported vehicle information PIDs
    if (vehicleInfoLength > 0) {
      Serial.print("VehicleInfo: ");
      for (int i = 0; i < vehicleInfoLength; i++) {
        byte supported = KLine.getSupportedData(0x09, i);  // Get supported vehicle information PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);
    // KLine.readSupportedData(0x01);
    // delay(1000);
  }
}
