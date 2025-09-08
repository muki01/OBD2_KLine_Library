#include "OBD2_KLine.h"  // Include the library for OBD2 K-Line communication
//#include <AltSoftSerial.h>  // Optional alternative software serial (not used here)
//AltSoftSerial Alt_Serial;   // Create an alternative serial object (commented out)

// ---------------- Create an OBD2_KLine object for communication.
OBD2_KLine KLine(Serial1, 10400, 10, 11);  // Uses Hardware Serial (Serial1) at 10400 baud, with RX on pin 10 and TX on pin 11.
//OBD2_KLine KLine(Alt_Serial, 10400, 8, 9); // Uses AltSoftSerial at 10400 baud, with RX on pin 8 and TX on pin 9.

void setup() {
  Serial.begin(115200);  // Start the default serial (for logging/debugging)
  Serial.println("OBD2 K-Line Get Freeze Frame Example");

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
    int rpm = KLine.getFreezeFrame(0x0C);  // PID 0x0C = Engine RPM
    Serial.print("Engine RPM: ");
    Serial.println(rpm);

    int coolantTemp = KLine.getFreezeFrame(0x05);  // PID 0x05 = Coolant Temperature
    Serial.print("Coolant Temp: ");
    Serial.print(coolantTemp);
    Serial.println(" C");

    // int speed = KLine.getPID(0x02, 0x0D); // Alternative
    int speed = KLine.getFreezeFrame(0x0D);  // PID 0x0D = Vehicle Speed
    Serial.print("Vehicle Speed: ");
    Serial.print(speed);
    Serial.println(" km/h");

    Serial.println();  // Print a blank line between readings
  }
}
