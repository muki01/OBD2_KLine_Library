#include "OBD2_KLine.h"  // Include the library for OBD2 K-Line communication
#include "Yamaha_F70.h"

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
#include <AltSoftSerial.h>
AltSoftSerial Alt_Serial;
OBD2_KLine KLine(Alt_Serial, 10400, 8, 9);  // Uses AltSoftSerial at 10400 baud, with RX on pin 8 and TX on pin 9.
#elif defined(ESP32)
OBD2_KLine KLine(Serial1, 10400, 10, 11);  // Uses Hardware Serial (Serial1) at 10400 baud, with RX on pin 10 and TX on pin 11.
#else
#error "Unsupported board! This library currently supports Arduino Uno, Nano, Mega, and ESP32. Please select a compatible board in your IDE."
#endif

int counter1 = 0;

void setup() {
  Serial.begin(115200);  // Start the default serial (for logging/debugging)

  KLine.setDebug(Serial);  // Optional: outputs debug messages to the selected serial port
  //KLine.setProtocol("ISO14230_Fast");  // Optional: communication protocol (default: Automatic; supported: ISO9141, ISO14230_Slow, ISO14230_Fast, Automatic)
  KLine.setByteWriteInterval(5);  // Optional: delay (ms) between bytes when writing
  KLine.setInterByteTimeout(60);  // Optional: sets the maximum inter-byte timeout (ms) while receiving data
  KLine.setReadTimeout(1000);     // Optional: maximum time (ms) to wait for a response after sending a request

  Serial.println("Yamaha Code.");
}

void loop() {
  Yamaha_Simulator();
}

void Yamaha_Simulator() {
  if (KLine.readData()) {
    if (KLine.compareData(yamahaInit, sizeof(yamahaInit))) {
      KLine.writeRawData(yamahaInit_Response1, sizeof(yamahaInit_Response1), 0);
      // delay(300);
      // writeRawData(yamahaEngineNO_Response, sizeof(yamahaEngineNO_Response), 0);
    }
    // if (KLine.resultBuffer[0] == 0xE5) {
      // delay(100);
      // KLine.writeRawData(yamahaLiveData1_Response, sizeof(yamahaLiveData1_Response), 0);
      // counter1++;
      // if (counter1 == 2) {
      //   delay(10);
      //   writeRawData(yamahaInit_Response2, sizeof(yamahaInit_Response2), 0);
      // }
    // }
  }
}
