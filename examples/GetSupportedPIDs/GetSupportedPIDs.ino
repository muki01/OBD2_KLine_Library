#include "OBD2_KLine.h"

OBD2_KLine KLine(Serial1, 10400, 10, 11);

void setup() {
  Serial.begin(115200);
  Serial.println("OBD2 K-Line Clear DTC Example");
  KLine.setDebug(Serial);

  KLine.setProtocol("ISO14230_Fast");
  KLine.setWriteDelay(5);
  KLine.setDataRequestInterval(60);
  KLine.beginSerial();
  Serial.println("OBD2 Starting.");
}

void loop() {
  if (KLine.initOBD2()) {
    int liveDataLength = KLine.readSupportedLiveData();
    if (liveDataLength > 0) {
      Serial.print("LiveData: ");
      for (int i = 0; i < liveDataLength; i++) {
        byte supported = KLine.getSupportedData(0x01, i);
        Serial.print(supported, HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);

    int freezeFrameLength = KLine.readSupportedFreezeFrame();
    if (freezeFrameLength > 0) {
      Serial.print("FreezeFrame: ");
      for (int i = 0; i < freezeFrameLength; i++) {
        byte supported = KLine.getSupportedData(0x02, i);
        Serial.print(supported, HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);

    int vehicleInfoLength = KLine.readSupportedVehicleInfo();
    if (vehicleInfoLength > 0) {
      Serial.print("VehicleInfo: ");
      for (int i = 0; i < vehicleInfoLength; i++) {
        byte supported = KLine.getSupportedData(0x09, i);
        Serial.print(supported, HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    delay(1000);
    // KLine.readSupportedData(0x01);
    // delay(1000);
  }
}
