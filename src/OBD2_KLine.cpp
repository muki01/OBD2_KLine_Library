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
void OBD2_KLine::writeData(const byte mode, const byte pid) {
  //debugPrintln("Writing...");
  byte message[7] = { 0 };
  size_t length = (mode == read_FreezeFrame) ? 7 : (mode == init_OBD || mode == read_DTCs || mode == clear_DTCs) ? 5
                                                                                                                 : 6;

  if (protocol == "ISO9141") {
    message[0] = (mode == read_FreezeFrame) ? 0x69 : 0x68;
    message[1] = 0x6A;
  } else if (protocol == "ISO14230_Fast" || protocol == "ISO14230_Slow") {
    message[0] = (mode == read_FreezeFrame) ? 0xC3 : (mode == init_OBD || mode == read_DTCs || mode == clear_DTCs) ? 0xC1
                                                                                                                   : 0xC2;
    message[1] = 0x33;
  }

  message[2] = 0xF1;
  message[3] = mode;
  if (length > 5) message[4] = pid;
  if (length == 7) message[5] = 0x00;

  message[length - 1] = calculateChecksum(message, length - 1);

  for (size_t i = 0; i < length; i++) {
    _serial.write(message[i]);
    delay(_writeDelay);
  }


byte OBD2_KLine::calculateChecksum(const byte data[], int length) {
  byte checksum = 0;
  for (int i = 0; i < length; i++) {
    checksum += data[i];
  }
  return checksum % 256;
}

}
