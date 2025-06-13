#ifndef OBD2_KLINE_H
#define OBD2_KLINE_H

#include <Arduino.h>

class OBD2_KLine  {
public:
  OBD2_KLine(HardwareSerial& serialPort, long baudRate, uint8_t rxPin, uint8_t txPin);

  void beginSerial();
private:
  HardwareSerial& _serial;
  long _baudRate;
  bool _customPins;
  uint8_t _rxPin;
  uint8_t _txPin;
};

#endif // OBD2_KLINE_H