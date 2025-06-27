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

## 🛠️Schematics for communication
#### This is the schematic with Transistors
<img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/Transistor%20Schematic.png" width=70%>

The **R6** resistor in this schematic is designed for **3.3V** microcontrollers. If you are using a **5V** microcontroller, you need to change the **R6** value to **5.3kΩ**.

Additionally, I have observed that many test devices use a **1kΩ** value for **R4**. However, according to the K-Line documentation, the recommended value for **R4** is **510Ω**. It is advisable to follow this value. That being said, using **1kΩ** for **R4** will not cause any issues in the circuit. However, if you prefer to adhere to the documentation, **510Ω** is the recommended value.

<br>

#### This is the schematic with L9637D
<img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/L9637D.png" width=70%>
