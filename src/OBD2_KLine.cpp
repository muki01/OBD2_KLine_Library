#include "OBD2_KLine.h"

OBD2_KLine::OBD2_KLine(HardwareSerial& serialPort, long baudRate, uint8_t rxPin, uint8_t txPin)
  : _serial(serialPort), _baudRate(baudRate), _customPins(true), _rxPin(rxPin), _txPin(txPin) {}

