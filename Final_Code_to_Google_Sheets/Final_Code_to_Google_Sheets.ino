#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

const String web_app_url = "***"; Enter the url from google app script

#define WIFI_SSID "****" // Your Wifi SSID/Name.
#define WIFI_PASSWORD "****" // Your WiFi Password.

int blocks[] = {4,5,6,8,9};
#define blocks_len  (sizeof(blocks) / sizeof(blocks[0]))

#define RST_PIN  0  // GPIO
#define SS_PIN   2  // GPIO
#define BUZZER   16 // GPIO
#define wifistatuson D1
#define wifistautsoff D2

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;      

int blockNum = 2;  

byte bufferLen = 25;
byte readBlockData[18];

const uint8_t fingerprint[20] = {0xD5, 0x3A, 0x80, 0xA6, 0x03, 0xB0, 0xE4, 0x36, 0x0E, 0x46, 0x7B, 0x36 ,0x45 ,0xCB, 0x50, 0x4C, 0xD6, 0x98, 0xCE, 0x59}; // Edit Fingerprint Here

void setup()

{
  Serial.begin(9600);

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);

  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    digitalWrite(D1, HIGH);
    delay(200);
    digitalWrite(D1, LOW);
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  digitalWrite(D2, HIGH);

  pinMode(BUZZER, OUTPUT);

  SPI.begin();
}

 void loop()
{  
  mfrc522.PCD_Init();
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}

  if ( ! mfrc522.PICC_ReadCardSerial()) {return;}

  Serial.println();
  Serial.println(F("Reading last data from RFID..."));  
  String fullURL = "", temp;
  for (byte i = 0; i < blocks_len; i++) {
    ReadDataFromBlock(blocks[i], readBlockData);
    if(i == 0){
      temp = String((char*)readBlockData);
      temp.trim();
      fullURL = "data" + String(i) + "=" + temp;
    }
    else{
      temp = String((char*)readBlockData);
      temp.trim();
      fullURL += "&data" + String(i) + "=" + temp;
    }
  }

  fullURL.trim();
  fullURL = web_app_url + "?" + fullURL;
  fullURL.trim();
  Serial.println(fullURL);

  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(3000);

  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));
    if (https.begin(*client, (String)fullURL)){
      Serial.print(F("[HTTPS] GET...\n"));
      int httpCode = https.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      
      digitalWrite(BUZZER, HIGH);
      delay(100);
      digitalWrite(BUZZER, LOW);
      delay(100);
      }
      else 
      {Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());}
      https.end();
      
      delay(1000);

    }
    else {
      Serial.printf("[HTTPS} Unable to connect\n");
    }
  }
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
  else {
    Serial.println("Authentication success");
  }
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {
    readBlockData[16] = ' ';
    readBlockData[17] = ' ';
    Serial.println("Block was read successfully");  
  }
}
