// NAME: PN5180-ISO15693-IFTTT.ino
// Author: Taimur Shahzad Gill
// Copyright (c) 2018 by Andreas Trappmann. All rights reserved.
// Copyright (c) 2019 by Dirk Carstensen.

// include the sensor libraries
#include <PN5180.h>
#include <PN5180ISO15693.h>
#include <WiFi.h>
#include <esp_task_wdt.h>

#define WDT_TIMEOUT 5 //5 seconds WDT

// ESP-32 <-> PN5180 pinout mapping
#define PN5180_NSS  12  // GPIO12
#define PN5180_BUSY 13  // GPIO13
#define PN5180_RST  14  // GPIO14

const int relayPin =  25 ;  // GPIO25
int relayState = LOW;   // relayState used to set the relayPin

const char* NAME;
const char* ID;

String Event_Name = "card_scanned";
String Key = "gC-i81iUdqR161EqgFtbr0Pu_3IE52w1sxE0P8tMtz0";

// Unique IFTTT URL Resource
String resource = "/trigger/" + Event_Name + "/with/key/" + Key;

const char* server = "maker.ifttt.com"; // Maker Webhooks IFTTT

// SSID and Password for WiFi Access
const char* ssid     = "Samsung";
const char* password = "12345670";

PN5180ISO15693 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);
uint8_t lastUid[8];

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

  // Connect to WiFi using the SSID and Password
  Serial.print("Connecting to: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  int timeout = 10 * 5; 
  while (WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("WiFi connected in: ");
  Serial.print(millis());
  Serial.print(", IP address: ");
  Serial.println(WiFi.localIP());
}

// ISO 15693 loop
void loop() {
  
  uint8_t uid[8];
  esp_task_wdt_reset();
  
  // Try to read a tag ID (or "get inventory" in ISO15693-speak)
  ISO15693ErrorCode rc = nfc.getInventory(uid);
  
  // If the result code was that a card had been read
  if (rc == ISO15693_EC_OK) {
    // If this is the same ID as we read last frame
    if (memcmp(uid, lastUid, 8) == 0) {
    relayState = LOW;
    digitalWrite(LED, relayState);
    }
    
    // If it's a different ID
    else {
      relayState = HIGH;
      digitalWrite(LED, relayState);
      
      Serial.println(F("----------------------------------"));
      Serial.print(F("New Card Detected!"));
      
      NAME = "Irfan Ahmed";
      ID = "F998E4ED";

      Serial.println();
      Serial.print("Attendace Marked for "); Serial.println(NAME);
      makeIFTTTRequest();
  
      delay(1000);  
   }
   
   // Update the array that keeps track of most recent ID
   memcpy(lastUid, uid, sizeof(lastUid[0]) * 8);
   delay(50); 
   
   }
  
  // If a card is not detected
  else {
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


// Make an HTTP request to the IFTTT web service
void makeIFTTTRequest() {
  Serial.print("Connecting to ");
  Serial.print(server);

  WiFiClient client;
  int retries = 5;
  while (!!!client.connect(server, 80) && (retries-- > 0)) {
    Serial.print(".");
  }
  Serial.println();
  if (!!!client.connected()) {
    Serial.println("Failed to Connect!");
  }

  Serial.print("Request resource: ");
  Serial.println(resource);

  // Name and RFID of the user
  String jsonObject = String("{\"value1\":\"") + NAME + "\",\"value2\":\"" + ID
                      + "\"}";

  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server);
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length());
  client.println();
  client.println(jsonObject);

  int timeout = 5 * 10; // 5 seconds
  while (!!!client.available() && (timeout-- > 0)) {
    delay(100);
  }
  if (!!!client.available()) {
    Serial.println("No Response.");
  }
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("\nShutting Down Connection.");
  client.stop();
}
