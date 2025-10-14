#include "OBD2_KLine.h"  // Include the library for OBD2 K-Line communication
#include "Opel_Vectra_B_2001_Codes.h"
//#include <AltSoftSerial.h>  // Optional alternative software serial (not used here)
//AltSoftSerial Alt_Serial;   // Create an alternative serial object (commented out)

// ---------------- Create an OBD2_KLine object for communication.
OBD2_KLine KLine(Serial1, 10400, 10, 11);  // Uses Hardware Serial (Serial1) at 10400 baud, with RX on pin 10 and TX on pin 11.
//OBD2_KLine KLine(Alt_Serial, 10400, 8, 9); // Uses AltSoftSerial at 10400 baud, with RX on pin 8 and TX on pin 9.

bool connectionStatus = false;

void setup() {
  Serial.begin(115200);  // Start the default serial (for logging/debugging)

  KLine.setDebug(Serial);  // Optional: outputs debug messages to the selected serial port
  //KLine.setProtocol("ISO14230_Fast");  // Optional: communication protocol (default: Automatic; supported: ISO9141, ISO14230_Slow, ISO14230_Fast, Automatic)
  KLine.setByteWriteInterval(5);  // Optional: delay (ms) between bytes when writing
  KLine.setInterByteTimeout(60);  // Optional: sets the maximum inter-byte timeout (ms) while receiving data
  KLine.setReadTimeout(1000);     // Optional: maximum time (ms) to wait for a response after sending a request

  Serial.println("Opel Code.");
}

void loop() {
  Opel_Vectra_Simulator();

  //Opel_Vectra_Simulator2();  //Instrument Cluster Simulator (Select 4800 Baud)
  //if (connectionStatus) KLine.writeRawData(instumentClusterKeepalive_Response, sizeof(instumentClusterKeepalive_Response), 0);
}

void Opel_Vectra_Simulator() {
  if (KLine.readData()) {
    if (KLine.compareData(engineInit0, sizeof(engineInit0)) || KLine.compareData(engineInit, sizeof(engineInit))) KLine.writeRawData(engineInit_Response, sizeof(engineInit_Response), 2);
    else if (KLine.compareData(engineCheckConnection, sizeof(engineCheckConnection))) KLine.writeRawData(engineCheckConnection_Response, sizeof(engineCheckConnection_Response), 2);
    else if (KLine.compareData(engineReadLiveData1, sizeof(engineReadLiveData1))) KLine.writeRawData(engineReadLiveData1_Response, sizeof(engineReadLiveData1_Response), 2);
    else if (KLine.compareData(engineReadLiveData2, sizeof(engineReadLiveData2))) KLine.writeRawData(engineReadLiveData2_Response, sizeof(engineReadLiveData2_Response), 2);
    else if (KLine.compareData(engineReadLiveData3, sizeof(engineReadLiveData3))) KLine.writeRawData(engineReadLiveData3_Response, sizeof(engineReadLiveData3_Response), 2);

    if (KLine.compareData(imoInit0, sizeof(imoInit0))) KLine.writeRawData(imoInit_Response, sizeof(imoInit_Response), 2);
    else if (KLine.compareData(imoKeepalive, sizeof(imoKeepalive))) KLine.writeRawData(imoKeepalive_Response, sizeof(imoKeepalive_Response), 2);
    else if (KLine.compareData(imoReadLiveData, sizeof(imoReadLiveData))) KLine.writeRawData(imoReadLiveData_Response, sizeof(imoReadLiveData_Response), 2);
    else if (KLine.compareData(imoReadDTCs, sizeof(imoReadDTCs))) KLine.writeRawData(imoReadDTCs_Response, sizeof(imoReadDTCs_Response), 2);
  }
}

void Opel_Vectra_Simulator2() {
  if (connectionStatus == false) {
    if (KLine.read5baud() == 0x60) {
      connectionStatus = true;
      KLine.writeRawData(instumentClusterInit_Response, sizeof(instumentClusterInit_Response), 0);
    }
  }
  if (connectionStatus == true) {
    if (KLine.readData()) {
      if (KLine.compareData(instumentClusterLiveData, sizeof(instumentClusterLiveData))) KLine.writeRawData(instumentClusterLiveData_Response, sizeof(instumentClusterLiveData_Response), 0);
      if (KLine.resultBuffer[0] == 0x7F) {}
    }
  }
}
