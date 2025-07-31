# OBD2 KLine Library

![GitHub forks](https://img.shields.io/github/forks/muki01/OBD2_KLine_Library?style=flat)
![GitHub Repo stars](https://img.shields.io/github/stars/muki01/OBD2_KLine_Library?style=flat)
![GitHub Issues or Pull Requests](https://img.shields.io/github/issues/muki01/OBD2_KLine_Library?style=flat)
![GitHub License](https://img.shields.io/github/license/muki01/OBD2_KLine_Library?style=flat)
![GitHub last commit](https://img.shields.io/github/last-commit/muki01/OBD2_KLine_Library)

**OBD2_KLine** is a lightweight yet powerful Arduino-compatible library that enables direct communication with vehicles using the **K-Line** (ISO 9141 / ISO 14230 - KWP2000).

This library is designed for microcontrollers such as **Arduino**, **ESP32**, and similar platforms. It allows your device to communicate directly with a vehicle that uses the **K-Line**.  
If you're curious about which types of data are supported, you can find a full list of features below.

K-Line is a legacy protocol used in many **European and Japanese vehicles manufactured between ~1987 and 2010**, especially before CAN became mandatory in newer models.  
This makes the library ideal for working with older cars that still rely on ISO9141 or KWP2000 communication standards.

You can also see my other car projects:
1. [Тhis](https://github.com/muki01/I-K_Bus) project is for BMW with I/K bus system. 
2. [Тhis](https://github.com/muki01/OBD2_CAN_Bus_Reader) project is for Cars with CAN Bus.
3. [Тhis](https://github.com/muki01/OBD2_K-line_Reader) project is for Cars with ISO9141 and ISO14230 protocols.
4. [Тhis](https://github.com/muki01/OBD2_CAN_Bus_Library) is my OBD2 CAN Bus Communication Library for Arduino IDE.

---

## ❓ Does Your Vehicle Support K-Line?

Before using this library, it's important to confirm whether your vehicle supports the **K-Line** protocol.

K-Line vehicles typically have **Pin 7** on the OBD-II connector connected.  
If your vehicle’s OBD-II connector has **Pins 6 and 14 connected**, it uses the **CAN bus** protocol instead of K-Line.

✅ **Pin 7 (K-Line)**: Your vehicle likely supports ISO 9141 or ISO 14230 (KWP2000) — this library will work.  
❌ **Pin 6 and 14 (CAN bus)**: Your vehicle uses CAN — consider a different library.

### Example photos of OBD2 Connector
<p>
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20KLine.jpg" width=40%>
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20CanBus.jpg" width=40%>
</p>

In the first image, the OBD2 socket includes pin 7, which indicates it operates using the K-Line protocol.
In the second image, pins 6 and 14 are present, meaning it uses the CAN Bus protocol.

---

## ✨ Features

- Supports **ISO 9141-2** and **ISO 14230-4 (KWP2000)**
- **5-baud initialization (slow init)** and **Fast init** support
- **Automatic protocol detection**
- Read real-time sensor values
- Read and clear **stored and pending DTCs**
- Retrieve vehicle info (VIN, calibration IDs, etc.)
- **Mode 06** support (on-board test results)
- Debug output for easier development
- Customizable delays and request intervals
- Works with Arduino, ESP32 and similar platforms

---

## 📡 Supported OBD-II Modes

| Mode | Description                                      |
|------|--------------------------------------------------|
| 01   | Read current live data (sensor values)           |
| 02   | Read freeze frame data                           |
| 03   | Read stored Diagnostic Trouble Codes (DTCs)      |
| 04   | Clear DTCs and MIL reset                         |
| 05   | Oxygen sensor test results                       |
| 06   | On-board monitoring test results                 |
| 07   | Read pending Diagnostic Trouble Codes            |
| 09   | Retrieve vehicle information (VIN, calibration)  |

---

### 📊 Typical Data Rates

Each protocol has its own timing characteristics, which affect how many responses you can expect per second when reading data from the ECU. The values below reflect **actual performance measurements** based on this library’s real-world testing.

| Protocol     | Average Responses per Second |
|--------------|-------------------------------|
| ISO 9141-2   | ~7–8 responses/sec            |
| ISO 14230-4  | ~8–9 responses/sec            |

> 🔎 Note: Tese values represent average conditions based on real-world testing. The actual throughput can vary depending on the ECU’s internal processing time, the specific data being requested (e.g. PID type), and system latency.

---

## 🛠️ Schematics for Communication

These schematics are essential because K-Line communication operates at different voltage and signal levels than microcontroller pins.  
The circuits ensure proper level shifting and protection for safe, stable operation.

You can choose one of the following approaches depending on your project:

### 🔹 Schematic with Transistors
<img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/Transistor%20Schematic.png" width=70%>

- **R6 3kΩ** is for **3.3V MCUs**. Use **5.3kΩ** for **5V** systems.
- **R4** is often used as **1kΩ**, but the K-Line standard recommends **510Ω**. Either option will work, but **510Ω** is more compliant with the standard.

### 🔹 Schematic with L9637D
<img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/L9637D.png" width=70%>

The **L9637D** is a dedicated K-Line transceiver chip that simplifies the interface circuit, reducing part count and improving signal reliability.

---
