// NAME: PN5180-ISO14443.ino
// Author: Taimur Shahzad Gill
// Copyright (c) 2018 by Andreas Trappmann. All rights reserved.
// Copyright (c) 2019 by Dirk Carstensen.

#include <PN5180.h>
#include <PN5180ISO15693.h>
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 2 //2 seconds WDT

// ESP-32 <-> PN5180 pinout mapping
#define PN5180_NSS  12  // GPIO12
#define PN5180_BUSY 13  // GPIO13
#define PN5180_RST  14  // GPIO14

const int relayPin =  25 ;  // GPIO25
int relayState = LOW;   // relayState used to set the relayPin

PN5180ISO15693 nfc(12, 13, 14);

void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  delay(500);  

  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  Serial.println(F("Uploaded: " __DATE__ " " __TIME__)); Serial.println(" ");

  nfc.begin();
  nfc.reset();
  nfc.setupRF();

  Serial.println(F("PN5180 ISO15693 WORKING!")); Serial.println(" ");
}

void loop() {

  uint8_t uid[8];
  
  esp_task_wdt_reset();

  // Try to read a tag ID (or "get inventory" in ISO15693-speak)
  ISO15693ErrorCode rc = nfc.getInventory(uid);
  
  // If the result code was that a card had been read
  if (rc == ISO15693_EC_OK) {
    
    relayState = HIGH;
    digitalWrite(relayPin, relayState);
    
    Serial.println(F("RFID Card Detected!"));
    
    delay(1000);  
    }
  
  // If a card is not detected
  else {
    // The most significant (last) byte of a valid UID should always be 0xE0. e.g. E007C4A509C247A8
    if (!rc == ISO15693_EC_OK) {
      
      relayState = LOW;
      digitalWrite(relayPin, relayState);
        
      Serial.println("Card removed from the reader!");
      delay(900);  
    }

    #ifdef DEBUG
    Serial.print(F("Error when reading : "));
    Serial.println(nfc.strerror(rc));
    #endif
  }

}
