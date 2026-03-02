#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include "RTClib.h"

#define CS_MATRIX   9 
#define CS_RFID     10 
#define RST_RFID    5
#define RED_LED     6

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_MATRIX, MAX_DEVICES);
MFRC522 rfid(CS_RFID, RST_RFID);
RTC_DS1307 rtc;

void setup() {
  Serial.begin(115200);
  
  // --- SPI INITIALIZATION OUTPUT ---
  Serial.println(F("\n=================================="));
  Serial.println(F("   SYSTEM BOOT & SPI INIT         "));
  Serial.println(F("=================================="));
  
  Wire.begin(); 
  
  // 1. Start the actual SPI Hardware Bus
  SPI.begin(); 
  Serial.println(F("[OK] SPI Master Bus Initialized"));
  
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW); 

  // 2. Initialize the Matrix over SPI
  myDisplay.begin();
  myDisplay.setIntensity(5); 
  myDisplay.displayClear();
  Serial.println(F("[OK] MAX7219 Matrix (SPI) Online"));

  // 3. Initialize the RFID over SPI
  rfid.PCD_Init(); 
  Serial.println(F("[OK] MFRC522 RFID (SPI) Online"));

  // I2C Device Init
  if (!rtc.begin()) {
    Serial.println(F("[ERROR] RTC not found on I2C bus!"));
  } else {
    Serial.println(F("[OK] DS1307 RTC (I2C) Online"));
  }
  
  // Comment this out after the first upload so the time doesn't freeze!
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  Serial.println(F("=================================="));
  Serial.println(F("   SYSTEM READY - AWAITING SCAN   "));
  Serial.println(F("==================================\n"));
}

void loop() {
  // --- CLOCK STANDBY ---
  DateTime now = rtc.now();
  char timeBuffer[6]; 
  sprintf(timeBuffer, "%02d:%02d", now.hour(), now.minute());

  if (myDisplay.displayAnimate()) {
    myDisplay.displayText(timeBuffer, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  }

  // --- SPI OPERATION: SENSOR POLLING ---
  // The Arduino is constantly asking the RFID over SPI if a card is there
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // --- EVENT TRIGGERED ---
  digitalWrite(RED_LED, HIGH);
  Serial.println(F("\n>>> EVENT: ATTENDANCE SCAN DETECTED <<<"));

  // --- SPI OPERATION DATA OUTPUT (READ) ---
  // We read the Unique ID from the card via the MFRC522 over SPI
  Serial.print(F("[SPI READ] MFRC522 UID Data (HEX): "));
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();

  // --- SPI OPERATION DATA OUTPUT (WRITE) ---
  Serial.println(F("[SPI WRITE] Sending Data to MAX7219: 'HELLO'"));
  myDisplay.displayClear();
  myDisplay.displayText("HELLO", PA_CENTER, 0, 1500, PA_PRINT, PA_NO_EFFECT);
  while(!myDisplay.displayAnimate()); 
  delay(1500); 

  // --- SPI OPERATION DATA OUTPUT (WRITE) ---
  Serial.println(F("[SPI WRITE] Sending Data to MAX7219: 'APRIL'"));
  myDisplay.displayClear();
  myDisplay.displayText("APRIL", PA_CENTER, 0, 1500, PA_PRINT, PA_NO_EFFECT);
  while(!myDisplay.displayAnimate()); 
  delay(1500);

  // --- RESET STATE ---
  digitalWrite(RED_LED, LOW);
  rfid.PICC_HaltA(); 
  rfid.PCD_StopCrypto1(); 
  
  Serial.println(F("[SYSTEM] Attendance Logged. Returning to Standby..."));
  Serial.println(F("----------------------------------\n"));
}