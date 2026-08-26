<div align="center">

# 🚗 OBD2 K-Line Library <br>(ISO 9141 · KWP2000 · KW1281 · DS2 · KW82)

**A professional, high-performance Arduino/ESP32 library for vehicle diagnostics over K-Line — universal OBD-II (ISO 9141-2, ISO 14230-4/KWP2000) plus manufacturer-specific deep access for VAG KW1281, BMW DS2 and Opel KW82.**

![GitHub forks](https://img.shields.io/github/forks/muki01/OBD2_KLine_Library?style=flat)
![GitHub Repo stars](https://img.shields.io/github/stars/muki01/OBD2_KLine_Library?style=flat)
![GitHub Issues or Pull Requests](https://img.shields.io/github/issues/muki01/OBD2_KLine_Library?style=flat)
![GitHub License](https://img.shields.io/github/license/muki01/OBD2_KLine_Library?style=flat)
![GitHub last commit](https://img.shields.io/github/last-commit/muki01/OBD2_KLine_Library)
![ESP32](https://img.shields.io/badge/ESP32-000000?logo=espressif&logoColor=red)
![Arduino](https://img.shields.io/badge/Arduino-00979D?logo=arduino&logoColor=white)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/muki01/library/OBD2%20K-Line.svg)](https://registry.platformio.org/libraries/muki01/OBD2%20K-Line)
[![Arduino IDE Library Manager](https://www.ardu-badge.com/badge/OBD2%20K-Line.svg)](https://www.ardu-badge.com/OBD2%20K-Line)

</div>

---

## 📌 Overview

**OBD2_KLine** is a professional, high-performance library for vehicle diagnostics via **K-Line**, supporting **ISO 9141-2, ISO 14230-4 / KWP2000, VAG KW1281, BMW DS2 and Opel KW82**. Designed for **Arduino, ESP32** and similar microcontrollers, it lets your device talk directly to a vehicle's ECU over the K-Line.

K-Line is a legacy protocol used in many **European and Japanese vehicles built between ~1987 and 2010**, especially before CAN became mandatory — making this library ideal for both **generic OBD-II diagnostics** and **manufacturer-specific deep system access** (VAG, BMW, Opel).

## ❓ Does Your Vehicle Support K-Line?

Confirm your car speaks K-Line by checking the OBD-II connector pins:

- ✅ **Pin 7 connected → K-Line** (ISO 9141 / ISO 14230). This library will work.
- ❌ **Pins 6 & 14 connected → CAN bus.** Use my [OBD2 CAN Bus Library](https://github.com/muki01/OBD2_CAN_Bus_Library) instead.

**Example OBD-II connectors** (left: K-Line with pin 7 · right: CAN with pins 6 & 14):

<p>
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20KLine.jpg" width="40%" alt="OBD2 Connector Pin 7 K-Line">
<img src="https://github.com/muki01/OBD2_KLine_Library/blob/main/images/OBD2%20CanBus.jpg" width="40%" alt="OBD2 Connector Pin 6 and 14 CAN Bus">
</p>

## 🚀 Key Features

- **Universal compatibility** — Arduino (Uno, Nano, Mega), ESP32 and other popular MCUs.
- **Six protocols, one API** — the packet format is a setting, not a rewrite of your sketch.

  | Category | Supported Protocols |
  | :--- | :--- |
  | **Universal OBD2** | ISO 9141-2, ISO 14230-4 (KWP2000) |
  | **VAG (VW/Audi/Seat/Škoda)** | **KW1281** (legacy block protocol) |
  | **BMW** | **DS2** (Diagnostic System 2) |
  | **Opel / Vauxhall** | **KW82** |
  | **Anything else** | **Custom** — define the framing yourself |

- **Flexible initialization** — **5-Baud (Slow Init)**, **Fast Init**, ping and handshake-free modes, each selectable independently of the protocol.
- **Automatic detection** — finds the protocol on its own when you do not yet know what the car speaks.
- **Handles the plumbing** — headers, length bytes, checksums and handshake timings are built and verified for you.
- **Developer friendly** — integrated debug output showing every byte on the bus.

## 🔍 What You Can Read & Control

### 🔹 Standard OBD-II — works on any compliant car

Generic diagnostics defined by **SAE J1979**. No car-specific configuration needed — plug in and read:

| Mode | Description |
| ---- | ----------- |
| 01 | Live data — real-time sensor values (RPM, coolant temp, speed, throttle, fuel trims…) |
| 02 | Freeze frame — the sensor snapshot stored when a fault appeared |
| 03 | Read stored Diagnostic Trouble Codes (DTCs) |
| 04 | Clear DTCs and reset the MIL |
| 05 | Oxygen sensor test results |
| 06 | On-board monitoring test results |
| 07 | Read pending Diagnostic Trouble Codes |
| 09 | Vehicle information — VIN, Calibration IDs, Calibration Verification Numbers |

DTCs come back as readable codes (`P0123` style), and a **supported-PID scan** lets you ask the ECU
which PIDs it actually implements before requesting them.

### 🔸 Manufacturer protocols — deeper, car-specific access

The manufacturer protocols reach data and functions that generic OBD-II never exposes:

- **Extended live data** — manufacturer measurement blocks carrying far more channels than the standard PIDs, decoded into named values with real units (injection time, ignition advance, idle actuator steps, engine load, lambda, and so on). One request returns the whole block, so a dozen values cost a single message.
- **Vehicle control & actuator tests** — command the ECU to drive real hardware: MIL and service lamps, fuel pump relay, A/C relay, throttle actuator, tank vent valve, and per-cylinder ignition coil or injection cut-off.
- **Full ECU identification** — VIN, part number, supplier, hardware version, software number and engine code.
- **ECU memory & flash reading** — read the ECU's internal memory block by block across the whole flash range, streamed over the debug port so you can capture the dump to a file on your PC for analysis.
- **Raw service access** — send any manufacturer service by hand; the library still adds the header, length byte and checksum for you.
- **Discovery tools** — scan which identifiers an ECU answers and dump responses as offset tables, so you can map an ECU nobody has documented yet.

> 💡 Manufacturer-specific access is provided through **ECU definition files**. The generic OBD-II layer ships with the library; car-specific definitions (Opel Simtec 71 / Bosch M1.5.5 / Bosch ME7.5, BMW Bosch BMS 46 and others) are maintained separately — see the **Contact** section below.

## 📊 Typical Data Rates

Real-world throughput measured with this library:

| Protocol | Average responses per second |
| -------- | ---------------------------- |
| ISO 9141-2 | ~8–9 responses/sec |
| ISO 14230-4 | ~9–10 responses/sec |

> 🔎 Actual throughput varies with the ECU's internal processing time, the requested PID type and system latency.

## 📦 Installation

### Arduino Library Manager (recommended)
1. Open the **Arduino IDE**.
2. Go to **Sketch → Include Library → Manage Libraries…**
3. Search for **"OBD2 K-Line"**.
4. Click **Install**.

### Manual
Download this repo as a `.zip` and add it via **Sketch → Include Library → Add .ZIP Library…**

## ⚡ Basic Usage — Read Live Data

Read Engine RPM, Coolant Temperature and Vehicle Speed. The library automatically handles different board architectures (AVR / ESP32):

```cpp
#include "OBD2_KLine.h"          // core: connection + protocol layer
#include "ecus/OBD2_Standard.h"  // standard OBD2 diagnostics -> OBD2_KLine

OBD2_KLine KLine;

// Uno / Nano have no spare hardware serial, so they fall back to AltSoftSerial
// on its fixed pins (RX 8, TX 9). Every other board uses Serial1.
#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  #include <AltSoftSerial.h>
  AltSoftSerial altSerial;
  #define OBD_SERIAL  altSerial
#else
  #define OBD_SERIAL  Serial1
#endif

#define OBD_RX_PIN  5
#define OBD_TX_PIN  4

void setup() {
  Serial.begin(115200);

  KLine.setSerial(OBD_SERIAL);
  KLine.setPins(OBD_RX_PIN, OBD_TX_PIN);

  KLine.setDebug(Serial);        // View communication logs
  KLine.setProtocol(Automatic);  // Automatic, ISO9141, ISO14230, KW1281, DS2, KW82, Custom

  Serial.println("OBD2 System Starting...");
}

void loop() {
  if (!KLine.isConnected() && !KLine.connect()) return;

  float rpm     = KLine.getLiveData(0x0C);  // PID 0x0C: Engine RPM
  float coolant = KLine.getLiveData(0x05);  // PID 0x05: Coolant Temp
  float speed   = KLine.getLiveData(0x0D);  // PID 0x0D: Vehicle Speed

  Serial.print("RPM: ");   Serial.println(rpm);
  Serial.print("Temp: ");  Serial.print(coolant); Serial.println(" C");
  Serial.print("Speed: "); Serial.print(speed);   Serial.println(" km/h");
}
```

## 🛠️ Schematics for Communication

K-Line operates at different voltage/signal levels than microcontroller pins. These circuits provide level shifting and protection for safe, stable operation. Pick the approach that suits your project:

### 🔹 Transistor-based
<img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/Transistor%20Schematic.png" width="70%">

Simple, low-cost discrete-transistor interface for basic builds and prototyping. **R6** is sized for **3.3V** MCUs — for a **5V** MCU, change **R6** to **5.3 kΩ**.

### 🔹 Comparator-based
<img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/Comparator.png" width="70%">

Uses a cheap comparator IC (e.g. **LM393**) for a clean digital level — better noise immunity and well-defined thresholds than the transistor design, at a slightly higher component count.

### 🔹 Dedicated automotive IC
<p align="start">
  <img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/L9637D.png" width="45%" alt="L9637D"/>
  <img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/MC33290.png" width="42%" alt="MC33290"/>
</p>
<p align="start">
  <img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/Si9241.png" width="43%" alt="Si9241"/>
  <img src="https://github.com/muki01/OBD2_K-line_Reader/blob/main/Schematics/SN65HVDA195.png" width="45%" alt="SN65HVDA195"/>
</p>

Purpose-built K-Line / ISO 9141 transceiver ICs (**L9637D, MC33290, Si9241, SN65HVDA195**, etc.) with built-in level shifting and protection — highest reliability, recommended for production-grade designs.

## 📷 Gallery

Custom PCBs designed for this library:

<img width="36%" src="https://github.com/user-attachments/assets/3a34b38d-cd39-4f5f-b4dd-d671399bff53" alt="OBD2 K-Line PCB 1"/>
<img width="39%" src="https://github.com/user-attachments/assets/1a794aea-b9b8-4cdd-bebb-17b25fe7fd7b" alt="OBD2 K-Line PCB 2"/>

> 🛠️ **Custom hardware & PCBs:** looking for ready-to-use devices or custom-made PCBs based on this project? Reach out via email in the **Contact** section below.

## 🔗 Related Projects

Part of a full OBD2 / automotive diagnostics ecosystem:

| Firmware & Readers | Libraries | Manufacturer Protocols | UI |
|--------------------|-----------|------------------------|-----|
| [OBD2 K-line Reader](https://github.com/muki01/OBD2_K-line_Reader) | [OBD2 K-Line Library](https://github.com/muki01/OBD2_KLine_Library) | [BMW I/K Bus](https://github.com/muki01/I-K_Bus) | [OBD2 Diagnostic UI](https://github.com/muki01/OBD2-Diagnostic-UI) |
| [OBD2 CAN Bus Reader](https://github.com/muki01/OBD2_CAN_Bus_Reader) | [OBD2 CAN Bus Library](https://github.com/muki01/OBD2_CAN_Bus_Library) | [VAG KW1281](https://github.com/muki01/VAG_KW1281) | |


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

<div align="center">

Created by [**Muki**](https://github.com/muki01) · If you find this useful, consider giving it a ⭐

</div>
