/*
 * OBD2_KLine Library - MukiTech
 * -----------------------------
 * A professional Arduino library for OBD2 communication via K-Line (ISO 9141-2 and ISO 14230-4).
 *
 * Developed by: Muksin Muksin
 * GitHub: https://github.com/muki01/OBD2_KLine_Library
 * Email: muksin.muksin04@gmail.com
 *
 * This library is designed for automotive diagnostics, supporting various
 * microcontrollers including Arduino AVR and ESP32.
 *
 * FILE STRUCTURE
 * --------------
 *   OBD2_KLine_Core.h/.cpp     KLine_Core - Microcontroller side only: serial port, pins,
 *                              sending a ready made packet / receiving data, single byte
 *                              access, the P1..P4 byte timings, the raw init signals
 *                              (5 baud pattern, fast init pulse) and the debug output.
 *                              This header depends on nothing but Arduino.h.
 *   KLine_Protocol.h/.cpp      KLine_Protocol - Protocol layer. The top of the header holds
 *                              the shared type vocabulary every layer speaks (OBD2Protocol,
 *                              OBD2InitType, OBD2Checksum, OBD2LengthMode, OBD2Parity) - these
 *                              describe what the bus looks like, so they belong here.
 *                              The top of the .cpp holds the default settings of EVERY
 *                              protocol (baud rate, checksum type, parity, header, length byte
 *                              mode, timings and which handshake it uses) as one table per
 *                              protocol: ISO9141, ISO14230 Slow/Fast, KW1281, UDS_KLine, DS2,
 *                              KW82. The rest is protocol selection, packet building (header,
 *                              length byte, checksum), the block framing used by KW1281, the
 *                              handshake settings (init address, init parity, W1..W4, wake-up
 *                              delay) and the connection logic (slow / fast init).
 *   KLine_Functions.h/.cpp     Shared helpers as plain free functions, no object and no state:
 *                              checksum arithmetic (XOR, Modulo256, Two's Complement),
 *                              byte array comparison (compareData) and conversion / decoding
 *                              (decodeDTC, isInArray, convertBytesToHexString,
 *                              convertHexToAscii). Any layer may call them.
 *   ecus/OBD2_Standard.h/.cpp  OBD2_Standard - Standard OBD2 diagnostics (generic 0x33 address):
 *                              live data, freeze frame, DTCs, vehicle info, supported PIDs.
 *                              Defined by SAE J1979, so it works on any car - but it lives
 *                              under "ecus/" like every other diagnostic vocabulary and is
 *                              only compiled when a sketch includes it.
 *   ecus/<Ecu>.h/.cpp          One self contained file pair per ECU. Holds its own connection
 *                              settings, its own service requests and its own response layout
 *                              (which byte means what). Manufacturer services are NOT shared
 *                              between ECUs on purpose - addresses, sub functions and
 *                              identifiers differ from car to car.
 *                              Include only the one you need:
 *                                #include "ecus/Simtec71.h"
 *
 * The core (this file and the three next to it) is always compiled. Everything under "ecus/"
 * is opt in: including this header alone gives you the connection, not a diagnostic
 * vocabulary. Pick the one you need:
 *
 *   #include "OBD2_KLine.h"
 *   #include "ecus/OBD2_Standard.h"   // standard OBD2, any car    -> OBD2_KLine
 *   #include "ecus/Simtec71.h"        // Opel / Vauxhall Simtec 71 -> Simtec71
 *
 * Unused ECU tables never reach the flash this way.
 *
 * LICENSE: DUAL-LICENSED
 * 1. PERSONAL/RESEARCH: Free for non-commercial use.
 * 2. COMMERCIAL: Mandatory paid license required for any for-profit usage.
 * Copyright (c) 2025 MukiTech. All rights reserved.
 */

#ifndef OBD2_KLINE_H
#define OBD2_KLINE_H

// The order matters: Core is the leaf, Protocol defines the shared enums and
// then pulls in Functions, which needs those enums.
#include "OBD2_KLine_Core.h"
#include "KLine_Protocol.h"
#include "KLine_Functions.h"

// The files under "ecus/" are deliberately NOT included here. Every sketch
// includes only the diagnostic vocabulary it uses, so unused tables never
// reach the flash:
//   #include "ecus/OBD2_Standard.h"   -> OBD2_KLine
//   #include "ecus/Simtec71.h"        -> Simtec71

#endif  // OBD2_KLINE_H
