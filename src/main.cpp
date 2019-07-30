#include <ArduinoJson.h>
#include <ArduinoBLE.h>
#include <uFire_EC_JSON.h>
#include <uFire_pH_JSON.h>
#include <uFire_ORP_JSON.h>

BLEService                uFire_Service("4805d2d0-af9f-42c1-b950-eae78304c408");
BLEStringCharacteristic   tx_Characteristic("50fa7d80-440a-44d2-967a-ec7731ec736a", BLENotify, 20);
BLEStringCharacteristic   rx_Characteristic("50fa7d80-440b-44d2-967b-ec7731ec736b", BLEWrite, 20);
uFire_EC_JSON             ec;
uFire_pH_JSON             ph;
uFire_ORP_JSON            orp;

void rxCallback(BLEDevice central, BLECharacteristic characteristic) {
  // get the command sent to the characteristic
  String rx_command = rx_Characteristic.value();

  // run the command through the uFire EC class and see if it matches. 
  // "" is returned if there is no match
  String json_out = ec.processJSON(rx_command);
  if (json_out != "")
  {
    // the command matched, was executed, and returned a value in json_out
    tx_Characteristic.setValue(json_out);

    // exit since there's nothing else to do
    return;
  }

  // try pH
  json_out = ph.processJSON(rx_command);
  if (json_out != "")
  {
    tx_Characteristic.setValue(json_out);
    return;
  }

   // try ORP
  json_out = orp.processJSON(rx_command);
  if (json_out != "")
  {
    tx_Characteristic.setValue(json_out);
    return;
  }
}

void setup() {
  ec.begin(new uFire_EC);
  ph.begin(new ISE_pH);
  orp.begin(new ISE_ORP(0x3e));
  
  Wire.begin();
  BLE.begin();
  BLE.setLocalName("uFire BLE");
  BLE.setAdvertisedService(uFire_Service);
  uFire_Service.addCharacteristic(tx_Characteristic);
  uFire_Service.addCharacteristic(rx_Characteristic);
  BLE.addService(uFire_Service);
  rx_Characteristic.setEventHandler(BLEWritten, rxCallback);
  BLE.advertise();
}

void loop() {
  BLE.poll();
}
