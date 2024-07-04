/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <Arduino.h>
// audio
#include <Audio.h>
#include <SD.h>
#include <FS.h>
// ble
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
// #include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

// define IO
#define SD_CS 18
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 5
#define I2S_DOUT 22
#define I2S_BCLK 26
#define I2S_LRC 25

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// global variables
Audio audio;

BLECharacteristic *pCharacteristic = NULL;

int wait_duration = 5000;

int count = 0;

int playing = true;

int last_play_time = millis();

// ble callbacks
class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    rxValue = rxValue.c_str();

    if (rxValue == "U")
    {
      wait_duration += 1000;
      Serial.print("Duration: ");
      Serial.println(wait_duration);
    }
    else if (rxValue == "D")
    {
      wait_duration -= 1000;
      Serial.print("Duration: ");
      Serial.println(wait_duration);
    }
    if (rxValue == "O")
    {
      wait_duration += 1000;
      Serial.print("On");
      playing = true;
    }
    else if (rxValue == "X")
    {
      wait_duration -= 1000;
      Serial.print("Off");
      playing = false;
    }
  }
};

void setup()
{
  Serial.begin(115200);

  // configure spi for sd
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SD.begin(SD_CS);
  Serial.println("sd connected");

  // configure audio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21);
  // audio.connecttoFS(SD, "/bark.wav");

  // create device as server and create service
  BLEDevice::init("Bird Box");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // create characteristic
  pCharacteristic =
      pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);

  // set callbacks
  pCharacteristic->setCallbacks(new MyCallbacks());

  // set value of characteristic
  pCharacteristic->setValue("Hello World says Neil");

  // Create a BLE Descriptor
  // pCharacteristic->addDescriptor(new BLE2902());

  // start device and advertising
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop()
{
  audio.loop();

  if (millis() - last_play_time > wait_duration && playing)
  {
    last_play_time = millis();
    audio.connecttoFS(SD, "/bark.wav");
  }
}