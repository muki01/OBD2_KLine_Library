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
    String VIN = KLine.getVehicleInfo(0x02);
    Serial.print("VIN: "), Serial.println(VIN);

    String CalibrationID = KLine.getVehicleInfo(0x04);
    Serial.print("CalibrationID: "), Serial.println(CalibrationID);
    
    String CalibrationID_Num = KLine.getVehicleInfo(0x06);
    Serial.print("CalibrationID_Num: "), Serial.println(CalibrationID_Num);

    Serial.println();
  }
}
