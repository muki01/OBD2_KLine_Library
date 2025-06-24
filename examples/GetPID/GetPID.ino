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
    int rpm = KLine.getLiveData(0x0C);
    Serial.print("Engine RPM: "), Serial.println(rpm);

    int coolantTemp = KLine.getLiveData(0x05);
    Serial.print("Coolant Temp: "), Serial.print(coolantTemp), Serial.println(" C");

    int speed = KLine.getLiveData(0x0D);  //KLine.getPID(0x01, 0x0D);
    Serial.print("Vehicle Speed: "), Serial.print(speed), Serial.println(" km/h");
    Serial.println();
  }
}
