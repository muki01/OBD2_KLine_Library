/*
 * KWP2000 - 5 Baud Slow Init to ECU Address 0x01
 *
 * Connects to an ECU with a 5 baud slow init on the PHYSICAL address 0x01
 * (instead of the functional OBD2 address 0x33) and then reads identification
 * data using manufacturer level KWP2000 services.
 *
 * This sketch uses NO ECU definition file. It talks to the protocol layer
 * directly: you supply the service bytes, the library adds the header, the
 * length byte and the checksum. That is the path to take when no ECU file
 * exists for your car yet.
 *
 * Protocol   : ISO14230 (KWP2000)
 * Baud Rate  : 10400 baud, 8N1
 * Init       : 5 baud, address 0x01
 * Header     : 80 01 F1  (format / target / source)
 * Checksum   : Modulo-256
 *
 * Three independent settings:
 *   setProtocol(ISO14230)      the packet format
 *   setInitType(Init_5Baud)    how the link is opened (ISO14230 defaults to fast init)
 *   setHeader(...)             who is being addressed
 * They are independent on purpose - the same protocol can be reached with a slow
 * init or a fast init, functionally or physically addressed.
 *
 * About the format byte:
 *   Bits 7,6 of the first header byte select the addressing mode, bits 5..0
 *   carry the message length.
 *     10xxxxxx ($80) = with address information, PHYSICAL addressing
 *     11xxxxxx ($C0) = with address information, FUNCTIONAL addressing
 *   The library default is $C0 because standard OBD2 talks functionally to
 *   address $33. Here we address one specific ECU (0x01), so this sketch
 *   overrides the header with setHeader() to use $80. The length is inserted
 *   automatically, so a 2 byte request goes out as $82 01 F1 ...
 *
 * Services used:
 *   $1A Read ECU Identification        (KWP2000 spec 3.7)
 *   $3E Tester Present                 (KWP2000 spec 3.26)
 *
 * Note: Which identification options ($1A) an ECU answers is manufacturer
 * specific. The ones below are the values defined in the KWP2000 specification -
 * an ECU answers "Request Out Of Range ($31)" or "Service Not Supported ($11)"
 * for the ones it does not implement, and this sketch simply skips those.
 */

#include "OBD2_KLine.h"

KLine_Protocol KLine;   // the protocol layer on its own - no ECU file needed

// #include <AltSoftSerial.h>
// AltSoftSerial altSerial;

// #include <SoftwareSerial.h>
// SoftwareSerial softSerial(10, 11);

#define OBD_SERIAL        Serial1
#define OBD_DEBUG_SERIAL  Serial
#define OBD_RX_PIN        10
#define OBD_TX_PIN        11

#define ECU_ADDRESS       0x01   // physical address of the ECU we are talking to
#define TESTER_ADDRESS    0xF1

#define SID_READ_ECU_ID   0x1A
#define SID_TESTER_PRESENT 0x3E
#define NEGATIVE_RESPONSE 0x7F

// $80 = with address information, physical addressing. The length goes into the
// lower 6 bits automatically, so this is really "8x 01 F1".
const uint8_t KWP_HEADER[] = { 0x80, ECU_ADDRESS, TESTER_ADDRESS };

// The identification options defined by the KWP2000 specification.
const uint8_t IDENT_OPTIONS[] = { 0x80, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x90, 0x91, 0x92, 0x9A };

bool readIdentification(uint8_t option);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  KLine.setSerial(OBD_SERIAL);
  KLine.setPins(OBD_RX_PIN, OBD_TX_PIN);

  // -- Optional Settings --
  KLine.setDebug(OBD_DEBUG_SERIAL);   // Enable debug output to Serial

  // Order matters: setProtocol() loads the protocol defaults, so everything
  // that overrides them has to come after it.
  KLine.setProtocol(ISO14230);        // 10400 baud, 8N1, KWP2000 framing
  KLine.setInitType(Init_5Baud);      // ISO14230 defaults to fast init - override it
  KLine.setInitAddress(ECU_ADDRESS);  // the 5 baud init is sent to this address
  KLine.setInitParity(Parity_Even);   // ISO14230 default; some ECUs want Parity_Odd
  KLine.setHeader(KWP_HEADER);        // $80 = physical addressing (see note above)

  Serial.println(F("=== KWP2000 | 5 Baud Slow Init ==="));
  Serial.println(F("Reading ECU identification ($1A)"));
  Serial.println(F("=================================="));
}

void loop() {
  if (!KLine.isConnected()) {
    Serial.println(F("Connecting to ECU..."));
    if (!KLine.connect()) {
      Serial.println(F("Connection failed. Retrying..."));
      delay(3000);
      return;
    }
    Serial.println(F("Connection established."));
    Serial.println(F("----------------------------------------"));

    for (uint8_t i = 0; i < sizeof(IDENT_OPTIONS); i++) {
      readIdentification(IDENT_OPTIONS[i]);
      delay(100);   // stay inside P3
    }
    Serial.println(F("----------------------------------------"));
  }

  KLine.writeData((uint8_t[]){SID_TESTER_PRESENT});   // keep the session alive
  KLine.readData();
  delay(2000);
}

// Sends $1A <option> and prints the answer as hex and as text.
//
// Response layout:  [format] [target] [source] ([length]) [SID] [data...] [checksum]
// The length is only a separate byte when the lower 6 bits of the format byte
// are zero - otherwise it is embedded in the format byte itself.
bool readIdentification(uint8_t option) {
  KLine.writeData((uint8_t[]){SID_READ_ECU_ID, option});

  uint8_t length = KLine.readData();
  if (length == 0) return false;

  const uint8_t* response = KLine.getResultBuffer();
  const uint8_t sidIndex = ((response[0] & 0x3F) == 0) ? 4 : 3;

  if (length < sidIndex + 2) return false;              // header + SID + checksum
  if (response[sidIndex] == NEGATIVE_RESPONSE) return false;  // not supported by this ECU
  if (response[sidIndex] != SID_READ_ECU_ID + 0x40) return false;

  // Skip the service ID and the echoed option byte, drop the checksum.
  const uint8_t dataStart = sidIndex + 2;
  if (length < dataStart + 1) return false;
  const uint8_t dataLength = length - dataStart - 1;

  Serial.print(F("$1A $"));
  if (option < 0x10) Serial.print('0');
  Serial.print(option, HEX);
  Serial.print(F("  ("));
  Serial.print(dataLength);
  Serial.println(F(" bytes)"));

  Serial.print(F("  hex : "));
  Serial.println(convertBytesToHexString(&response[dataStart], dataLength));
  Serial.print(F("  text: "));
  Serial.println(convertHexToAscii(&response[dataStart], dataLength));

  return true;
}
