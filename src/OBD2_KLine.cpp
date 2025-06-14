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

  clearEcho();
}

void OBD2_KLine::clearEcho() {
  int result = _serial.available();
  if (result > 0) {
    //debugPrint("Cleared Echo Data: ");
    for (int i = 0; i < result; i++) {
      //debugPrintHex(_serial.read());
      //debugPrint(" ");
    }
    //debugPrintln();
  } else {
    //debugPrintln("Not Received Echo Data");
  }
}

byte OBD2_KLine::calculateChecksum(const byte data[], int length) {
  byte checksum = 0;
  for (int i = 0; i < length; i++) {
    checksum += data[i];
  }
  return checksum % 256;
}

void OBD2_KLine::setWriteDelay(uint16_t delay) {
  _writeDelay = delay;
}


void OBD2_KLine::send5baud(uint8_t data) {
  byte even = 1;  // parity bit hesaplama için
  byte bits[10];

  bits[0] = 0;  // start bit
  bits[9] = 1;  // stop bit

  // 7-bit data ve parity hesaplama
  for (int i = 1; i <= 7; i++) {
    bits[i] = (data >> (i - 1)) & 1;
    even ^= bits[i];
  }

  bits[8] = (even == 0) ? 1 : 0;  // parity biti

  //debugPrint("5 Baud Init for Module 0x");
  //debugPrintHex(data);
  //debugPrint(": ");

  // Pin'i OUTPUT yapmayı unutma (özellikle custom pin kullanılıyorsa)
  pinMode(_txPin, OUTPUT);

  for (int i = 0; i < 10; i++) {
    //debugPrint(bits[i] ? "1" : "0");
    digitalWrite(_txPin, bits[i] ? HIGH : LOW);
    delay(200);
  }

  //debugPrintln();
}
