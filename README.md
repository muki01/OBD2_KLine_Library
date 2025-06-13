# OBD2 KLine Library

This is Arduino library for communicating with vehicles over the **OBD2 K-Line** interface, using **ISO 9141** and **ISO 14230 (KWP2000)** protocols.

It allows you to connect your Arduino and ESP boards to your car's ECU (Engine Control Unit) and read diagnostic data like DTCs (Diagnostic Trouble Codes), live sensor data, and more.

---

## 📦 Features

- OBD-II communication over K-Line
- Supports ISO9141 and ISO14230 protocols
- Read DTCs, live data, freeze frame, and more
- 5-baud init and fast init supported
- Works with Arduino boards (e.g., UNO, Mega)
- Works with ESP boards (e.g., ESP32, ESP8266)

---

## 🔌 Wiring

Connect the K-Line pin (usually pin 7 on the OBD2 port) through a suitable transceiver (like L9637 or similar) to your Arduino or ESP device's RX and TX pins.
