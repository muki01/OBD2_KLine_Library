#include "OBD2_KLine.h"

OBD2_KLine KLine(Serial1, 10400, 10, 11);

void setup() {
  Serial.begin(115200);
  Serial.println("OBD2 K-Line PID Reading Example");
  KLine.setDebug(Serial);

  KLine.setProtocol("ISO14230_Fast");
  KLine.setWriteDelay(5);
  KLine.setDataRequestInterval(60);
  KLine.beginSerial();
  Serial.println("OBD2 Starting.");
}

void loop() {
  if (KLine.initOBD2()) {
    int storedDTCLength = KLine.readStoredDTCs();
    //int storedDTCLength = KLine.readDTCs(0x03);
    if (storedDTCLength > 0) {
      for (int i = 0; i < storedDTCLength; i++) {
        String dtc = KLine.getStoredDTC(i);
        Serial.println(dtc);
      }
      Serial.println();
    } else {
      Serial.println("No Stored DTCs");
    }

    int pendingDTCLength = KLine.readPendingDTCs();
    //int pendingDTCLength = KLine.readDTCs(0x07);
    if (pendingDTCLength > 0) {
      for (int i = 0; i < pendingDTCLength; i++) {
        String dtc = KLine.getPendingDTC(i);
        Serial.println(dtc);
      }
      Serial.println();
    } else {
      Serial.println("No Pending DTCs");
    }
  }
}
