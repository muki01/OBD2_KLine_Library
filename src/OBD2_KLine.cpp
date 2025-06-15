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

bool OBD2_KLine::init_OBD2() {
  if (protocol == "Automatic" || protocol == "ISO14230_Slow" || protocol == "ISO9141") {
    //debugPrintln("Trying ISO9141 or ISO14230_Slow");

    _serial.end();
    pinMode(_rxPin, INPUT_PULLUP);
    pinMode(_txPin, OUTPUT);
    digitalWrite(_txPin, HIGH);
    delay(3000);

    send5baud(0x33);

    beginSerial();

    if (readData()) {
      if (resultBuffer[0] == 0x55) {
        if (resultBuffer[1] == resultBuffer[2]) {
          //debugPrintln("Your Protocol is ISO9141");
          protocol = "ISO9141";
        } else {
          //debugPrintln("Your Protocol is ISO14230_Slow");
          protocol = "ISO14230_Slow";
        }
        //debugPrintln("Writing KW2 Reversed");
        _serial.write(~resultBuffer[2]);  // 0xF7

        if (readData()) {
          if (resultBuffer[0]) {
            return true;
          } else {
            //debugPrintln("No Data Retrieved from Car");
          }
        }
      }
    }
  }

  if (protocol == "Automatic" || protocol == "ISO14230_Fast") {
    //debugPrintln("Trying ISO14230_Fast");

    _serial.end();
    pinMode(_rxPin, INPUT_PULLUP);
    pinMode(_txPin, OUTPUT);
    digitalWrite(_txPin, HIGH);
    delay(3000);
    digitalWrite(_txPin, LOW);
    delay(25);
    digitalWrite(_txPin, HIGH);
    delay(25);

    beginSerial();

    writeData(init_OBD, 0x00);

    if (readData()) {
      if (resultBuffer[3] == 0xC1) {
        //debugPrintln("Your Protocol is ISO14230_Fast");
        protocol = "ISO14230_Fast";
        return true;
      }
    }
  }

  //debugPrintln("No Protocol Found");
  return false;
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

bool OBD2_KLine::readData() {
  //debugPrintln("Reading...");
  unsigned long startMillis = millis();
  int bytesRead = 0;

  // İlk byte için 1 saniye bekle
  while (millis() - startMillis < 1000) {
    if (_serial.available() > 0) {
      unsigned long lastByteTime = millis();
      memset(resultBuffer, 0, sizeof(resultBuffer));
      errors = 0;

      // İç döngü: bütün veriyi oku
      //debugPrint("Received Data: ");
      while (millis() - lastByteTime < _dataRequestInterval) {  // 60ms boyunca yeni veri bekle
        if (_serial.available() > 0) {                          // Yeni veri varsa
          if (bytesRead >= sizeof(resultBuffer)) {              // Buffer dolarsa dur
            //debugPrintln("\nBuffer is full. Stopping data reception.");
            return true;
          }
          resultBuffer[bytesRead] = _serial.read();
          //debugPrintHex(resultBuffer[bytesRead]);
          //debugPrint(" ");
          bytesRead++;
          lastByteTime = millis();  // Timer'ı resetle
        }
      }

      //debugPrintln("\nData reception completed.");
      return true;
    }
  }

  // 1 saniyede veri gelmezse
  //debugPrintln("Timeout: Not Received Data.");
  errors++;
  if (errors > 2) {
    errors = 0;
    if (conectionStatus) {
      conectionStatus = false;
    }
  }
  return false;
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

void OBD2_KLine::setDataRequestInterval(uint16_t interval) {
  _dataRequestInterval = interval;
}

void OBD2_KLine::setProtocol(const String& protocolName) {
  protocol = protocolName;
  //debugPrintln(("Protocol set to: " + protocol).c_str());
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
