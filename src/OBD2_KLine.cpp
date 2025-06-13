#include "OBD2_KLine.h"

OBD2_KLine::OBD2_KLine(HardwareSerial& serialPort, long baudRate, uint8_t rxPin, uint8_t txPin)
  : _serial(serialPort), _baudRate(baudRate), _customPins(true), _rxPin(rxPin), _txPin(txPin) {}

void OBD2_KLine::beginSerial() {
  if (_customPins) {
    _serial.begin(_baudRate, SERIAL_8N1, _rxPin, _txPin);
  } else {
    _serial.begin(_baudRate);
  }
}
