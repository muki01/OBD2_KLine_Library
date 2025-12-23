#include "OBD2_KLine.h"  // Include the library for OBD2 K-Line communication
#include "Honda_CR-V_2003.h"

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

  KLine.setDebug(Serial);  // Optional: outputs debug messages to the selected serial port
  //KLine.setProtocol("ISO14230_Fast");  // Optional: communication protocol (default: Automatic; supported: ISO9141, ISO14230_Slow, ISO14230_Fast, Automatic)
  KLine.setByteWriteInterval(5);  // Optional: delay (ms) between bytes when writing
  KLine.setInterByteTimeout(60);  // Optional: sets the maximum inter-byte timeout (ms) while receiving data
  KLine.setReadTimeout(1000);     // Optional: maximum time (ms) to wait for a response after sending a request

  Serial.println("Honda Code.");
}

void loop() {
  Honda_Simulator();
}

void Honda_Simulator() {
  if (KLine.readData()) {
    if (KLine.compareData(hondaEngine, sizeof(hondaEngine))) KLine.writeRawData(hondaEngine_Response, sizeof(hondaEngine_Response), 0);
    else if (KLine.compareData(hondaEngine2, sizeof(hondaEngine2))) KLine.writeRawData(hondaEngine_Response, sizeof(hondaEngine_Response), 0);
    else if (KLine.compareData(hondaEngine_LiveData1, sizeof(hondaEngine_LiveData1))) KLine.writeRawData(hondaEngine_LiveData1_R, sizeof(hondaEngine_LiveData1_R), 0);
    else if (KLine.compareData(hondaEngine_LiveData2, sizeof(hondaEngine_LiveData2))) KLine.writeRawData(hondaEngine_LiveData2_R, sizeof(hondaEngine_LiveData2_R), 3);
    else if (KLine.compareData(hondaEngine_LiveData3, sizeof(hondaEngine_LiveData3))) KLine.writeRawData(hondaEngine_LiveData3_R, sizeof(hondaEngine_LiveData3_R), 0);
    else if (KLine.compareData(hondaEngine_LiveData4, sizeof(hondaEngine_LiveData4))) KLine.writeRawData(hondaEngine_LiveData4_R, sizeof(hondaEngine_LiveData4_R), 0);
    else if (KLine.compareData(hondaEngine_LiveData5, sizeof(hondaEngine_LiveData5))) KLine.writeRawData(hondaEngine_LiveData5_R, sizeof(hondaEngine_LiveData5_R), 0);
    else if (KLine.compareData(hondaEngine_LiveData6, sizeof(hondaEngine_LiveData6))) KLine.writeRawData(hondaEngine_LiveData6_R, sizeof(hondaEngine_LiveData6_R), 0);
    else if (KLine.compareData(hondaEngine_LiveData7, sizeof(hondaEngine_LiveData7))) KLine.writeRawData(hondaEngine_LiveData7_R, sizeof(hondaEngine_LiveData7_R), 0);
    else if (KLine.compareData(hondaEngine_LiveData8, sizeof(hondaEngine_LiveData8))) KLine.writeRawData(hondaEngine_LiveData8_R, sizeof(hondaEngine_LiveData8_R), 0);
    else if (KLine.compareData(hondaEngine_LiveData9, sizeof(hondaEngine_LiveData9))) KLine.writeRawData(hondaEngine_LiveData9_R, sizeof(hondaEngine_LiveData9_R), 0);
    else if (KLine.compareData(hondaEngine_ReadDTCs, sizeof(hondaEngine_ReadDTCs))) KLine.writeRawData(hondaEngine_ReadDTCs_R, sizeof(hondaEngine_ReadDTCs_R), 0);
    else if (KLine.compareData(hondaEngine_ClearDTCs, sizeof(hondaEngine_ClearDTCs))) KLine.writeRawData(hondaEngine_ClearDTCs_R, sizeof(hondaEngine_ClearDTCs_R), 0);
    //else if (KLine.compareData(hondaABS, sizeof(hondaABS))) KLine.writeRawData(hondaABS_Response, sizeof(hondaABS_Response), false);
  }
}
