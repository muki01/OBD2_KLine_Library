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
  Serial.println("OBD2 K-Line Get Live Data Example");

  KLine.setDebug(Serial);          // Optional: outputs debug messages to the selected serial port
  KLine.setProtocol("Automatic");  // Optional: communication protocol (default: Automatic; supported: ISO9141, ISO14230_Slow, ISO14230_Fast, Automatic)
  KLine.setByteWriteInterval(5);   // Optional: delay (ms) between bytes when writing
  KLine.setInterByteTimeout(60);   // Optional: sets the maximum inter-byte timeout (ms) while receiving data
  KLine.setReadTimeout(1000);      // Optional: maximum time (ms) to wait for a response after sending a request

  KLine.setInitAddress(0x33);                 // Optional: Sets the target ECU address used during the 5-baud Slow Init sequence.
  KLine.setISO9141Header(0x68, 0x6A, 0xF1);   // Optional: Configures the 3-byte header (Priority, Receiver, Transmitter) for ISO9141.
  KLine.setISO14230Header(0xC0, 0x33, 0xF1);  // Optional: Configures the 3-byte header (Format, Receiver, Transmitter) for KWP2000.
  KLine.setLengthMode(true);                  // Optional: Defines if data length is embedded in the header or sent as a separate byte.

  Serial.println("OBD2 Starting.");
}

void loop() {
  // Attempt to initialize OBD2 communication
  if (KLine.initOBD2()) {
    int rpm = KLine.getLiveData(0x0C);  // PID 0x0C = Engine RPM
    Serial.print("Engine RPM: ");
    Serial.println(rpm);

    int coolantTemp = KLine.getLiveData(0x05);  // PID 0x05 = Coolant Temperature
    Serial.print("Coolant Temp: ");
    Serial.print(coolantTemp);
    Serial.println(" C");

    // int speed = KLine.getPID(0x01, 0x0D); // Alternative
    int speed = KLine.getLiveData(0x0D);  // PID 0x0D = Vehicle Speed
    Serial.print("Vehicle Speed: ");
    Serial.print(speed);
    Serial.println(" km/h");

    Serial.println();  // Print a blank line between readings
  }
}
