#include "OBD2_KLine.h"  // Include the library for OBD2 K-Line communication
#include "Opel_Vectra_B_2001_Codes.h"

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
#include <AltSoftSerial.h>
AltSoftSerial Alt_Serial;
OBD2_KLine KLine(Alt_Serial, 10400, 8, 9);  // Uses AltSoftSerial at 10400 baud, with RX on pin 8 and TX on pin 9.
#elif defined(ESP32)
OBD2_KLine KLine(Serial1, 10400, 10, 11);  // Uses Hardware Serial (Serial1) at 10400 baud, with RX on pin 10 and TX on pin 11.
#else
#error "Unsupported board! This library currently supports Arduino Uno, Nano, Mega, and ESP32. Please select a compatible board in your IDE."
#endif

bool connectionStatus = false;
int kw81_Stage = 0;

void setup() {
  Serial.begin(115200);  // Start the default serial (for logging/debugging)

  KLine.setDebug(Serial);              // Optional: outputs debug messages to the selected serial port
  KLine.setProtocol("ISO14230_Fast");  // Optional: communication protocol (default: Automatic; supported: ISO9141, ISO14230_Slow, ISO14230_Fast, Automatic)
  KLine.setByteWriteInterval(5);       // Optional: delay (ms) between bytes when writing
  KLine.setInterByteTimeout(60);       // Optional: sets the maximum inter-byte timeout (ms) while receiving data
  KLine.setReadTimeout(1000);          // Optional: maximum time (ms) to wait for a response after sending a request

  //KLine.setInitAddress(0x33);                 // Optional: Sets the target ECU address used during the 5-baud Slow Init sequence.
  //KLine.setISO9141Header(0x68, 0x6A, 0xF1);   // Optional: Configures the 3-byte header (Priority, Receiver, Transmitter) for ISO9141.
  KLine.setISO14230Header(0x80, 0x11, 0xF1);  // Optional: Configures the 3-byte header (Format, Receiver, Transmitter) for KWP2000.
  //KLine.setLengthMode(true);                  // Optional: Defines if data length is embedded in the header or sent as a separate byte.
  KLine.setChecksumType(2);                   // Optional: Selects checksum method (0: None, 1: XOR, 2: Modulo 256, 3: Two's Complement).

  Serial.println("Opel Code.");
}

void loop() {
  //KLine.read5baud();
  Opel_Vectra_Test1();
  //Opel_Vectra_Test2();

  //Opel_Vectra_Simulator1();  //Engine and Immobilizer
  //Opel_Vectra_Simulator2();  //Instrument Cluster Simulator (Select 4800 Baud)
}

void Opel_Vectra_Test1() {
  if (KLine.initOBD2()) {
    KLine.writeRawData(engineReadLiveData1, 2), KLine.readData();
    KLine.writeRawData(engineReadLiveData2, 2), KLine.readData();
    KLine.writeRawData(engineReadLiveData3, 2), KLine.readData();

    // KLine.writeRawData(imoReadLiveData, 2), KLine.readData();
    // KLine.writeRawData(imoReadDTCs, 2), KLine.readData();
    // KLine.writeRawData(imoClearDTCs, 2), KLine.readData();
  }
}

void Opel_Vectra_Test2() {
  if (KLine.initOBD2()) {
    KLine.readData();
  }
}


void Opel_Vectra_Simulator1() {
  if (KLine.readData()) {
    if (KLine.compareData(engineInit0) || KLine.compareData(engineInit)) KLine.writeRawData(engineInit_Response, 2);
    else if (KLine.compareData(engineCheckConnection)) KLine.writeRawData(engineCheckConnection_Response, 2);
    else if (KLine.compareData(engineReadLiveData1)) KLine.writeRawData(engineReadLiveData1_Response, 2);
    else if (KLine.compareData(engineReadLiveData2)) KLine.writeRawData(engineReadLiveData2_Response, 2);
    else if (KLine.compareData(engineReadLiveData3)) KLine.writeRawData(engineReadLiveData3_Response, 2);

    if (KLine.compareData(imoInit0)) KLine.writeRawData(imoInit_Response, 2);
    else if (KLine.compareData(imoKeepalive)) KLine.writeRawData(imoKeepalive_Response, 2);
    else if (KLine.compareData(imoReadLiveData)) KLine.writeRawData(imoReadLiveData_Response, 2);
    else if (KLine.compareData(imoReadDTCs)) KLine.writeRawData(imoReadDTCs_Response, 2);
    else if (KLine.compareData(imoClearDTCs)) KLine.writeRawData(imoClearDTCs_Response2, 2);
  }
}

void Opel_Vectra_Simulator2() {
  if (connectionStatus == false) {
    if (KLine.read5baud() == 0x60) {
      connectionStatus = true;
      KLine.writeRawData(instumentClusterInit_Response, 0);
    }
  }
  if (connectionStatus == true) {
    if (kw81_Stage == 0) KLine.writeRawData(instumentClusterECUID_Response, 0);
    else if (kw81_Stage == 1) KLine.writeRawData(instumentClusterLiveData_Response, 0);
    else if (kw81_Stage == 2) {}

    if (KLine.readData()) {
      if (KLine.compareData(instumentClusterLiveData)) kw81_Stage = 1;
      else if (KLine.compareData(instumentClusterClearDTC)) kw81_Stage = 2;
    }
  }
}
