#include "OBD2_KLine.h"

OBD2_KLine KLine(Serial1, 10400, 10, 11);

bool connected = false;

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
  if (!connected) {
    connected = KLine.init_OBD2();
  } else {
    int dtcLength = KLine.readDTCs();
    if (dtcLength > 0) {
      for (int i = 0; i < dtcLength; i++) {
        String dtc = KLine.getDTC(i);
        Serial.println(dtc);
      }
      Serial.println();
    }
  }
}
