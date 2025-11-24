/**
 * Classe WifiClientWrapper
 * 
 * 
 */

#ifndef WifiClientWrapper_H
#define WifiClientWrapper_H
#include "sys/_types.h"
#include <WiFi.h>
#include <WiFiManager.h>

#define TOKEN "estacaoteste"  // dados do thingsboard / TOKEN do canal thingsboard
#define THINGSBOARD_SERVER "15.229.47.249"
//#define THINGSBOARD_SERVER "https://thingsboard.ebt.synology.me"

#define MASK 0x400000000  // === 2^34 in hex https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
WiFiManager wifiManager;
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", THINGSBOARD_SERVER, 40);
WiFiManagerParameter custom_mqtt_token("token", "mqtt token", TOKEN, 20);


class WifiClientWrapper {

private:
  int resetWifiManagerPin = 34;
  unsigned long long bitMask = 0x400000000;  // bitmask pin 34
  int status = WL_IDLE_STATUS;               // status conexão WiFi
  String FirmwareVer = { "6.0" };            // versão do firmware

public:
  String mqttServerURL;
  String mqttDeviceToken;
  WifiClientWrapper();
  WifiClientWrapper(int sResetWifiManagerPin, unsigned long long bitMask);
  void setup();
  bool connectWifi();
  void markToReset();
  void reconnect();
  void resetSettings();
  void setupInterruptionHandler(void (*func)());
  void printWifiInfo();

};

WifiClientWrapper::WifiClientWrapper() {
  setup();
}


WifiClientWrapper::WifiClientWrapper(int sResetWifiManagerPin, unsigned long long sBitMask) {
  resetWifiManagerPin = sResetWifiManagerPin;
  bitMask = sBitMask;
  setup();
}

void WifiClientWrapper::setup() {
  wifiManager.setClass("invert");
  wifiManager.setConfigPortalTimeout(300);
  wifiManager.setTimeout(180);
  wifiManager.setCleanConnect(true);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_token);
  pinMode(resetWifiManagerPin, INPUT_PULLDOWN);
  esp_sleep_enable_ext1_wakeup(bitMask, ESP_EXT1_WAKEUP_ANY_HIGH);  // fator do pluviometro
}

void WifiClientWrapper::setupInterruptionHandler(void (*func)()) {
  attachInterrupt(digitalPinToInterrupt(resetWifiManagerPin), func, RISING);
}


bool WifiClientWrapper::connectWifi() {
  //Cria um AP (Access Point) com: ("nome da rede", "senha da rede")
  if (!wifiManager.autoConnect("estacao_meteorologica_net")) {
    Serial.println(F("Falha na conexao. Resetar e tentar novamente..."));
    delay(500);
    return false;
  }
  //read updated parameters
  mqttServerURL = custom_mqtt_server.getValue();
  mqttDeviceToken = custom_mqtt_token.getValue();
  //printWifiInfo();
  return true;
}

void WifiClientWrapper::resetSettings() {  //reeconexão em caso de queda
  Serial.println("Reseting wifi configs...");
  wifiManager.resetSettings();
}

void WifiClientWrapper::printWifiInfo() {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
#endif