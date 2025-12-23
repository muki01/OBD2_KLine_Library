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
  Serial.println("OBD2 K-Line Get Supported PIDs Example");

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
    int liveDataLength = KLine.readSupportedLiveData();  // Read supported live data PIDs. Mode: 01
    if (liveDataLength > 0) {
      Serial.print("LiveData: ");
      for (int i = 0; i < liveDataLength; i++) {
        byte supported = KLine.getSupportedData(0x01, i);  // Get supported live data PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.print("LiveData not supported!");
    }
    delay(1000);

    int freezeFrameLength = KLine.readSupportedFreezeFrame();  // Read supported freeze frame PIDs. Mode: 02
    if (freezeFrameLength > 0) {
      Serial.print("FreezeFrame: ");
      for (int i = 0; i < freezeFrameLength; i++) {
        byte supported = KLine.getSupportedData(0x02, i);  // Get supported freeze frame PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.print("FreezeFrame not supported!");
    }
    delay(1000);

    int oxygenSensorsLength = KLine.readSupportedOxygenSensors();  // Read supported Oxygen Sensors PIDs. Mode: 05
    if (oxygenSensorsLength > 0) {
      Serial.print("Oxygen Sensors: ");
      for (int i = 0; i < oxygenSensorsLength; i++) {
        byte supported = KLine.getSupportedData(0x05, i);  // Get supported components PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.print("Oxygen Sensors not supported!");
    }
    delay(1000);

    int otherComponentsLength = KLine.readSupportedOtherComponents();  // Read supported components PIDs. Mode: 06
    if (otherComponentsLength > 0) {
      Serial.print("Other Components: ");
      for (int i = 0; i < otherComponentsLength; i++) {
        byte supported = KLine.getSupportedData(0x06, i);  // Get supported components PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.print("Other Components not supported!");
    }
    delay(1000);

    int onBoardComponentsLength = KLine.readSupportedOnBoardComponents();  // Read supported On-Board Components PIDs. Mode: 08
    if (onBoardComponentsLength > 0) {
      Serial.print("On-Board Components: ");
      for (int i = 0; i < onBoardComponentsLength; i++) {
        byte supported = KLine.getSupportedData(0x08, i);  // Get supported components PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.print("On-Board Components not supported!");
    }
    delay(1000);

    int vehicleInfoLength = KLine.readSupportedVehicleInfo();  // Read supported vehicle information PIDs. Mode: 09
    if (vehicleInfoLength > 0) {
      Serial.print("VehicleInfo: ");
      for (int i = 0; i < vehicleInfoLength; i++) {
        byte supported = KLine.getSupportedData(0x09, i);  // Get supported vehicle information PID
        Serial.print(supported, HEX);                      // Print the PID in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.print("VehicleInfo not supported!");
    }
    delay(1000);
    // KLine.readSupportedData(0x01);
    // delay(1000);
  }
}
