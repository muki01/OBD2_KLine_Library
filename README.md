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
5. [Тhis](https://github.com/muki01/OBD2_KLine_Library) is my OBD2 K-Line Communication Library for Arduino IDE.
6. [Тhis](https://github.com/muki01/VAG_KW1281) project is for VAG Cars with KW1281 protocol.
<!--7. [Тhis](https://github.com/muki01/I-K_Bus_Library) is my I/K Bus Communication Library for Arduino IDE.-->

---

## ❓ Does Your Vehicle Support K-Line?

Before using this library, it's important to confirm whether your vehicle supports the **K-Line** protocol.

K-Line vehicles typically have **Pin 7** on the OBD-II connector connected.  
If your vehicle’s OBD-II connector has **Pins 6 and 14 connected**, it uses the **CAN bus** protocol instead of K-Line.

✅ **Pin 7 (K-Line)**: Your vehicle likely supports ISO 9141 or ISO 14230 (KWP2000) — this library will work.  
❌ **Pin 6 and 14 (CAN bus)**: Your vehicle uses CAN — consider a different library.

### Example photos of OBD2 Connector
<p>
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20KLine.jpg" width=40% alt="OBD2 Connector Pin 7 K-Line">
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20CanBus.jpg" width=40% alt="OBD2 Connector Pin 6 and 14 CAN Bus">
</p>

In the first image, the OBD2 socket includes pin 7, which indicates it operates using the K-Line protocol.
In the second image, pins 6 and 14 are present, meaning it uses the CAN Bus protocol.

---

## 🚀 Key Features

- **Universal Compatibility:** Works with Arduino (Uno, Nano, Mega), ESP32, and other popular microcontrollers.
- **Protocol Mastery:** Supports both **ISO 9141-2** and **ISO 14230-4 (KWP2000)**.
- **Advanced Initialization:** Features both **5-Baud (Slow Init)** and **Fast Init** methods.
- **Smart Detection:** Automatic protocol detection logic.
- **Full Diagnostics:** - Read real-time sensor data (PIDs).
  - Retrieve and Clear Stored/Pending DTCs (Diagnostic Trouble Codes).
  - Access Vehicle Info (VIN, Calibration IDs).
  - Monitor Oxygen Sensor and On-board test results.
- **Developer Friendly:** Integrated debug output for real-time monitoring.

---

## 📡 Supported OBD-II Modes
The table below lists the **standard OBD-II modes** supported by this library. These are universal for most vehicles.

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

> 💡 **Advanced Users:** These modes are standard for OBD2. However, if you have documentation for **manufacturer-specific PIDs** or custom communication commands for your specific vehicle, you can use the library's core functions to send those custom requests as well.

---

### 📊 Typical Data Rates

Each protocol has its own timing characteristics, which affect how many responses you can expect per second when reading data from the ECU. The values below reflect **actual performance measurements** based on this library’s real-world testing.

| Protocol     | Average Responses per Second |
|--------------|-------------------------------|
| ISO 9141-2   | ~7–8 responses/sec            |
| ISO 14230-4  | ~8–9 responses/sec            |

> 🔎 Note: Tese values represent average conditions based on real-world testing. The actual throughput can vary depending on the ECU’s internal processing time, the specific data being requested (e.g. PID type), and system latency.

---

## 🛠️Schematics for communication

These schematics are essential because K-Line communication operates at different voltage and signal levels than microcontroller pins.
The circuits ensure proper level shifting and protection for safe, stable operation.

You can choose one of the following approaches depending on your project:

### 🔹 Transistor-Based Schematic
<img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/Transistor%20Schematic.png" width=70%>

This schematic uses a discrete transistor-based approach to interface the K-Line with a microcontroller.
It is a simple and low-cost solution suitable for basic implementations and prototyping.

The **R6** resistor in this schematic is designed for **3.3V** microcontrollers. If you are using a **5V** microcontroller, you need to change the **R6** value to **5.3kΩ**.

### 🔹 Comparator-Based Schematic
<img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/Comparator.png" width=70%>

This design uses a low-cost comparator IC to process the K-Line signal and convert it into a clean digital level for the microcontroller.
It offers a good balance between cost, simplicity, and signal reliability.

- Can be implemented using cheap and widely available comparators such as LM393
- Better noise immunity than discrete transistor-based designs
- Provides well-defined logic thresholds
- Suitable for low-budget projects that require improved signal stability
- Slightly higher component count compared to the transistor solution, but still cost-effective

### 🔹 Dedicated Automotive IC Schematic
<p align="start">
  <img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/L9637D.png" width="45%" alt="L9637D"/>
  <img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/MC33290.png" width="42%" alt="MC33290"/>
</p>

<p align="start">
  <img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/Si9241.png" width="43%" alt="SI9241"/>
  <img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/SN65HVDA195.png" width="45%" alt="SN65HVDA195"/>
</p>


This schematic category uses dedicated automotive communication ICs (e.g. L9637D, MCZ33290, Si9241, SN65HVDA195 etc.) specifically designed for K-Line / ISO 9141 applications.

- Built-in voltage level shifting and protection
- Fully compliant with automotive communication standards
- Highest reliability and signal stability
- Recommended for production-grade and long-term use designs

---

## 📦 Installation

### Arduino Library Manager (Recommended)
1. Open the **Arduino IDE**.
2. Go to **Sketch** -> **Include Library** -> **Manage Libraries...**
3. Search for **"OBD2 K-Line"**.
4. Click **Install**.

### Manual Installation
Alternatively, download this repo as a `.zip` file and include it via **Sketch** -> **Include Library** -> **Add .ZIP Library...**

---

## ⚡ Basic Usage: Get Live Data

This example demonstrates how to read Engine RPM, Coolant Temperature, and Vehicle Speed. The library automatically handles different board architectures (AVR/ESP32).

```cpp
#include "OBD2_KLine.h"

// Check for board compatibility and define Serial interface
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
  #include <AltSoftSerial.h>
  AltSoftSerial Alt_Serial;
  OBD2_KLine KLine(Alt_Serial, 10400, 8, 9); // AVR: RX 8, TX 9
#elif defined(ESP32)
  OBD2_KLine KLine(Serial1, 10400, 10, 11); // ESP32: RX 10, TX 11
#endif

void setup() {
  Serial.begin(115200);
  
  KLine.setDebug(Serial);           // View communication logs
  KLine.setProtocol("Automatic");  // Supports ISO9141, ISO14230_Slow, ISO14230_Fast
  
  Serial.println("OBD2 System Starting...");
}

void loop() {
  if (KLine.initOBD2()) {
    int rpm = KLine.getLiveData(0x0C);        // PID 0x0C: Engine RPM
    int coolant = KLine.getLiveData(0x05);    // PID 0x05: Coolant Temp
    int speed = KLine.getLiveData(0x0D);      // PID 0x0D: Vehicle Speed

    Serial.print("RPM: "); Serial.println(rpm);
    Serial.print("Temp: "); Serial.print(coolant); Serial.println(" C");
    Serial.print("Speed: "); Serial.print(speed); Serial.println(" km/h");
  }
}

```

## 📷 Gallery
Custom PCBs designed for this library:

<img width=36% src="https://github.com/user-attachments/assets/3a34b38d-cd39-4f5f-b4dd-d671399bff53" alt="OBD2 K-Line PCB 1"/>
<img width=39% src="https://github.com/user-attachments/assets/1a794aea-b9b8-4cdd-bebb-17b25fe7fd7b" alt="OBD2 K-Line PCB 2"/>

> 🛠️ **Custom Hardware & PCBs:** If you are looking for ready-to-use devices or custom-made PCBs based on this project, feel free to reach out to me via email in the **Contact** section below.

---

## ☕ Support My Work

If you enjoy my projects and want to support me, you can do so through the links below:

[![Buy Me A Coffee](https://img.shields.io/badge/-Buy%20Me%20a%20Coffee-FFDD00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=black)](https://www.buymeacoffee.com/muki01)
[![PayPal](https://img.shields.io/badge/-PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://www.paypal.com/donate/?hosted_button_id=SAAH5GHAH6T72)
[![GitHub Sponsors](https://img.shields.io/badge/-Sponsor%20Me%20on%20GitHub-181717?style=for-the-badge&logo=github)](https://github.com/sponsors/muki01)

---

## 📬 Contact

For information, job offers, collaboration, sponsorship, or purchasing my devices, you can contact me via email.

📧 Email: muksin.muksin04@gmail.com

---
