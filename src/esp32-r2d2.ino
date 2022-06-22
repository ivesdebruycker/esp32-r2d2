/*
  ESP 32 - Sphero R2D2
*/

// https://github.com/nkolban/esp32-snippets/issues/757
// https://gist.github.com/sam016/4abe921b5a9ee27f67b3686910293026
// neilkolban.com/esp32/docs/cpp_utils/html/class_b_l_e_remote_characteristic.html#aa00bffdf3b2843aee32a27af9b643da5

// https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_client/BLE_client.ino
// https://github.com/igbopie/spherov2.js/blob/9a9fee843a9da6b26c14c60a93870ba242f03566/lib/src/toys/core.ts
// https://community.sphero.com/t/sphero-mini-not-responding-to-wake-command/1401
// https://medium.com/swlh/reverse-engineering-sphero-r2d2-with-javascript-8d1133269623
// https://cryptii.com/pipes/integer-encoder
// https://qiita.com/poruruba/items/634671131619f8067734
// https://github.com/marcos69/esp32bleSpheroMini
// https://lang-ship.com/reference/ESP32/latest/_b_l_e_remote_descriptor_8cpp_source.html
// http://www.neilkolban.com/esp32/docs/cpp_utils/html/class_b_l_e_remote_characteristic.html

// Advertised Device: Name: D2-52D5, Address: c2:80:9f:74:52:d5, serviceUUID: 00010001-574f-4f20-5370-6865726f2121

// Service: uuid: 00001800-0000-1000-8000-00805f9b34fb, start_handle: 1 0x0001, end_handle: 7 0x0007, Generic Access
// Service: uuid: 00001801-0000-1000-8000-00805f9b34fb, start_handle: 8 0x0008, end_handle: 11 0x000b, Generic Attribute
// Service: uuid: 0000180f-0000-1000-8000-00805f9b34fb, start_handle: 22 0x0016, end_handle: 25 0x0019, Battery Service
// Service: uuid: 00010001-574f-4f20-5370-6865726f2121, start_handle: 26 0x001a, end_handle: 65535 0xffff
// Service: uuid: 00020001-574f-4f20-5370-6865726f2121, start_handle: 12 0x000c, end_handle: 21 0x0015

// Dependencies
// |-- <WiFi> 1.0 (C:\Users\ives\.platformio\packages\framework-arduinoespressif32\libraries\WiFi)
// |-- <ESP32 BLE Arduino> 1.0.1 (C:\Users\ives\.platformio\packages\framework-arduinoespressif32\libraries\BLE)
// |-- <WebServer> 1.0 (C:\Users\ives\.platformio\packages\framework-arduinoespressif32\libraries\WebServer)

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5; //In seconds
BLEScan *pBLEScan;
static BLEUUID serviceUUID("00010001-574f-4f20-5370-6865726f2121");
static BLEAddress *Server_BLE_Address;
static BLERemoteCharacteristic *pCommandCharacteristic;

static BLEUUID commandServiceUUID("00010001-574f-4f20-5370-6865726f2121");
static BLEUUID dfuServiceUUID("00020001-574f-4f20-5370-6865726f2121");

static BLEUUID commandCharUUID("00010002-574f-4f20-5370-6865726f2121");
static BLEUUID dfuControlCharUUID("00020002-574F-4F20-5370-6865726F2121");
static BLEUUID dfuInfoCharUUID("00020004-574F-4F20-5370-6865726F2121");
static BLEUUID antiDoSCharUUID("00020005-574f-4f20-5370-6865726f2121");
static BLEUUID batteryServiceUUID("0000180f-0000-1000-8000-00805f9b34fb");
static BLEUUID batteryCharUUID("00002a19-0000-1000-8000-00805f9b34fb");

int seq = 0;
std::string generateCommand(char DID, char CID, char SEQ, char DLEN, ...)
{
  short i = 0;
  char sum = 0, data;
  va_list args;

  // Calculate checksum
  sum = DID + CID + SEQ + DLEN;

  int cnter = 6;
  char yyy[cnter + DLEN];
  yyy[0] = char(0xFF);
  yyy[1] = char(0xFE);
  yyy[2] = char(DID);
  yyy[3] = char(CID);
  yyy[4] = char(SEQ);
  yyy[5] = char(DLEN);

  va_start(args, DLEN);
  for (; i < DLEN - 1; i++)
  {
    data = va_arg(args, int);
    //Serial1.write(data);
    yyy[cnter] = data;
    cnter += 1;
    sum += data;
  }
  va_end(args);

  yyy[cnter] = char(~sum);

  return std::string(yyy);

  // Wait for Simple Response
  //return readSimplePacket();
}

void sendCommand(uint8_t *pData, int length)
{
  pCommandCharacteristic->writeValue(pData, length, false);
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    Serial.println(advertisedDevice.getAddress().toString().c_str());
    //Server_BLE_Address = new BLEAddress(advertisedDevice.getAddress());
    //Scaned_BLE_Address = Server_BLE_Address->toString().c_str();
  }
};

static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.println((char *)pData);
}

bool connectToserver(BLEAddress pAddress)
{
  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  // Connect to the BLE Server.
  pClient->connect(pAddress, BLE_ADDR_TYPE_RANDOM);
  Serial.print(" - Connected to R2D2: ");
  Serial.println(pClient->getPeerAddress().toString().c_str());

  Serial.print("rssi = ");
  Serial.println(pClient->getRssi());

  if (!pClient->isConnected())
  {
    Serial.println("Not connected");
    return false;
  }

  Serial.println(" - Getting services");
  std::map<std::string, BLERemoteService *> *foundServices = pClient->getServices();
  for (auto myPair = foundServices->begin(); myPair != foundServices->end(); myPair++)
  {
    /*std::string str = myPair->second->canRead()?"R":"";
      str += myPair->second->canWrite()?"\\W":"";
      str += myPair->second->canBroadcast()?"\\B":"";
      str += myPair->second->canIndicate()?"\\I":"";
      str += myPair->second->canNotify()?"\\N]":"]";*/
    //Serial.println(myPair->first.c_str());
    Serial.println(myPair->second->toString().c_str());
  }

  BLERemoteService *pDfuService = pClient->getService(dfuServiceUUID);
  if (pDfuService != nullptr)
  {
    BLERemoteCharacteristic *antiDoSCharacteristic = pDfuService->getCharacteristic(antiDoSCharUUID);
    if (antiDoSCharacteristic == nullptr)
    {
      Serial.print("Failed to find antiDoSCharacteristic");
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found antiDoSCharacteristic");

    uint8_t useTheForceMsg[] = {0x75, 0x73, 0x65, 0x74, 0x68, 0x65, 0x66, 0x6f, 0x72, 0x63, 0x65, 0x2e, 0x2e, 0x2e, 0x62, 0x61, 0x6e, 0x64};
    antiDoSCharacteristic->writeValue(useTheForceMsg, 18, false);
  }

  BLERemoteService *pBatteryService = pClient->getService(batteryServiceUUID);
  if (pBatteryService != nullptr)
  {
    Serial.println(" - Found batteryService");
    BLERemoteCharacteristic *pBatteryCharacteristic = pBatteryService->getCharacteristic(batteryCharUUID);
    if (pBatteryCharacteristic == nullptr)
    {
      Serial.println("Failed to find batteryCharacteristic");
      std::map<std::string, BLERemoteCharacteristic *> *pfoundCharacteristics = pBatteryService->getCharacteristics();
      for (auto myPair = pfoundCharacteristics->begin(); myPair != pfoundCharacteristics->end(); myPair++)
      {
        Serial.println(myPair->second->toString().c_str());
      }
    }
    else
    {
      Serial.println(" - Found batteryCharacteristic");
      if (pBatteryCharacteristic->canRead())
      {
        //std::string value = pBatteryCharacteristic->readValue();
        uint8_t value2 = pBatteryCharacteristic->readUInt8();
        Serial.print("Battery: ");
        Serial.print(value2);
        Serial.println("%");
        //Serial.println(value.c_str());
      }
      std::map<std::string, BLERemoteDescriptor *> *pfoundDescriptors = pBatteryCharacteristic->getDescriptors();
      for (auto myPair = pfoundDescriptors->begin(); myPair != pfoundDescriptors->end(); myPair++)
      {
        Serial.println(myPair->second->toString().c_str());
      }
    }
  }

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pCommandService = pClient->getService(commandServiceUUID);
  if (pCommandService != nullptr)
  {
    Serial.println(" - Found our service");
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pCommandCharacteristic = pCommandService->getCharacteristic(commandCharUUID);
    if (pCommandCharacteristic == nullptr)
    {
      Serial.print("Failed to find our characteristic UUID ");
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");
    if (pCommandCharacteristic->canWrite())
    {
      Serial.println("Writeable");
    }

    Serial.println("Wake");
    sendCommand(new uint8_t[7]{0x8d, 0x0a, 0x13, 0x0d, 0x00, 0xd5, 0xd8}, 7);
    //uint8_t wakeMsg[] = {0x8d, 0x0a, 0x13, 0x0d ,0x00 ,0xd5 ,0xd8};
    //pCommandCharacteristic->writeValue(wakeMsg, 7,false);
    delay(5000);

    Serial.println("set volume");
    uint8_t volumeMsg[] = {0x8d, 0x0a, 0x1a, 0x08, 0x01, 0x64, 0x6e, 0xd8};
    pCommandCharacteristic->writeValue(volumeMsg, 8, false);
    delay(500);

    Serial.println("set holo");
    uint8_t setR2D2HoloProjectorIntensityFullMsg[] = {0x8d, 0x0a, 0x1a, 0x0e, 0x02, 0x00, 0x80, 0xff, 0x4c, 0xd8};
    pCommandCharacteristic->writeValue(setR2D2HoloProjectorIntensityFullMsg, 10, false);
    delay(5000);

    Serial.println("Audio");
    sendCommand(new uint8_t[10]{0x8d, 0x0a, 0x1a, 0x07, 0x03, 0x02, 0x06, 0x00, 0xc9, 0xd8}, 10);
    delay(5000);

    Serial.println("Audio");
    sendCommand(new uint8_t[10]{0x8d, 0x0a, 0x1a, 0x07, 0x04, 0x0d, 0x5e, 0x00, 0x65, 0xd8}, 10);
    delay(5000);

    Serial.println("Audio");
    sendCommand(new uint8_t[10]{0x8d, 0x0a, 0x1a, 0x07, 0x05, 0x0d, 0x84, 0x00, 0x3e, 0xd8}, 10);
    delay(5000);

    /*uint8_t anim3Msg[] = {0x8d, 0x0a, 0x17, 0x05 ,0x02 ,0x00, 0x03, 0xd4 ,0xd8};
      pCommandCharacteristic->writeValue(anim3Msg, 9, false);
      Serial.println("anim3Msg");

      delay(5000);*/

    uint8_t anim5Msg[] = {0x8d, 0x0a, 0x17, 0x05, 0x03, 0x00, 0x05, 0xd1, 0xd8};
    pCommandCharacteristic->writeValue(anim5Msg, 9, false);
    Serial.println("anim5Msg");

    delay(5000);

    Serial.println("Sound");
    uint8_t soundMsg[] = {0x8d, 0x0a, 0x1a, 0x07, 0x01, 0x0a, 0x1a, 0x00, 0xaf, 0xd8};
    pCommandCharacteristic->writeValue(soundMsg, false);

    Serial.println("Sleep");
    uint8_t tst4Msg[] = {0x8d, 0x0a, 0x13, 0x01, 0x11, 0xd0, 0xd8};
    pCommandCharacteristic->writeValue(tst4Msg, 7, false);

    // Read the value of the characteristic.
    /*
      if(pCommandCharacteristic->canRead()) {
        std::string value = pCommandCharacteristic->readValue();
        Serial.print("The characteristic value was: ");
        Serial.println(value.c_str());
      }
      */

    /*
      if(pCommandCharacteristic->canNotify())
        pCommandCharacteristic->registerForNotify(notifyCallback);
      */

    return true;
  }
  else
  {
    return false;
  }

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  /*pCommandCharacteristic = pCommandService->getCharacteristic(charUUID);
    if (pCommandCharacteristic != nullptr)
      Serial.println(" - Found our characteristic");
      return true;*/
}

int LED_BUILTIN = 2;
boolean paired = false;

void setup()
{
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Test");
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Scanning...");
  BLEDevice::init("");

  /*
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  */
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  // put your main code here, to run repeatedly:
  /*
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(2000);
  */

  if (paired == false)
  {
    Serial.println("Connecting to Server as client");
    Server_BLE_Address = new BLEAddress("00:00:00:00:00:00");
    if (connectToserver(*Server_BLE_Address))
    {
      paired = true;
      Serial.println("Paired");
    }
    else
    {
      Serial.println("Pairing failed");
    }
  }

  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  delay(1000);                    // wait for a second
}
