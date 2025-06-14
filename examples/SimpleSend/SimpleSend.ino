#include <OBD2_KLine.h>

OBD2_KLine kline(Serial1, 10400, 9, 10);

void setup() {
  //kline.begin();
  //kline.setProtocol("ISO14230_Fast");
  //kline.setWriteDelay(5);
  //kline.setDataRequestInterval(60);
  while(!kline.init_OBD2()){

  }
}

void loop() {
  kline.writeData(read_LiveData, 0x0C);
  delay(1000);
}
