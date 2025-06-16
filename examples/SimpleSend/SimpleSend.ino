#include "OBD2_KLine.h"

OBD2_KLine kline(Serial1, 10400, 10, 11);

bool connected = false;

void setup() {
  Serial.begin(115200);
  Serial.println("OBD2 K-Line PID Reading Example");

  kline.setProtocol("ISO14230_Fast");
  kline.setWriteDelay(5);
  kline.setDataRequestInterval(60);
  kline.beginSerial();
  Serial.println("OBD2 Starting.");
}

void loop() {
  if (!connected) {
    Serial.println("Trying to connect.");
    connected = kline.init_OBD2();
  } else {
    int rpm = kline.getPID(0x0C);
    Serial.print("Engine RPM: "), Serial.println(rpm);

    int coolantTemp = kline.getPID(0x05);
    Serial.print("Coolant Temp: "), Serial.print(coolantTemp), Serial.println(" C");

    int speed = kline.getPID(0x0D);
    Serial.print("Vehicle Speed: "), Serial.print(speed), Serial.println(" km/h");
  }
}
