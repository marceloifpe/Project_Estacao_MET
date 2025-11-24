#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "Adafruit_Si7021.h"

//Libs Anemometro, Anemoscópio e Bateria
#include "Anemometro.h"
#include "Anemoscopio.h"
#include "Pluviometer.h"
#include "Bateria.h"
#include "WifiClientWrapper.h"
#include "ThingsBoardWrapper.h"
#include "driver/rtc_io.h"

#define SERIAL_DEBUG_BAUD 115200
const long long uS_TO_S_FACTOR =    1000000;  // Fator de conversão de micro segundos para segundos
const long long TIME_TO_SLEEP  =   3600;      // tempo de sleep em segundos


constexpr char CURRENT_FIRMWARE_TITLE[]   = "est_met_firm";
constexpr char CURRENT_FIRMWARE_VERSION[] = "2.4.8";
constexpr char CURRENT_CLIENT_ID[]        = "est_met_01";

//  ----------------------------------------------------------------------------------- //
const unsigned int resetWifiManagerPin  = 34;
const unsigned long long bitMaskPin     = 0x400000000;   // 2^34 in hex https://randomnerdtutorials.com/esp32-external-wake-up-deep-sleep/
const int anemoscopioPins[8]            = { 4, 21, 13, 27, 33,  //Pins do ESP32
                                          15, 32, 14 };
const unsigned int anemoSensorPin       = 25;
const gpio_num_t gpioRainSensorPin      = GPIO_NUM_26;


Adafruit_BMP280 bmpSensor;  //objetos de comunicação com os sensores
Adafruit_Si7021 siSensor = Adafruit_Si7021();
Anemometro anemometro(anemoSensorPin);  //Objeto de comunicação anemômetro, anemoscópio e bateria
Anemoscopio anemoscopio(anemoscopioPins);
Pluviometer pluvi(gpioRainSensorPin);
Bateria bateria;
WifiClientWrapper wifiControl(resetWifiManagerPin, bitMaskPin);
ThingsBoardWrapper clientMQTT(CURRENT_FIRMWARE_TITLE, CURRENT_FIRMWARE_VERSION, CURRENT_CLIENT_ID);
ThingsBoardWrapper clientMQTT_elton(CURRENT_FIRMWARE_TITLE, CURRENT_FIRMWARE_VERSION, CURRENT_CLIENT_ID);

volatile uint32_t tempoUltimaInterrupcaoAnem  = 0;
volatile uint32_t tempoUltimaInterrupcaoPluvi = 0;
volatile uint32_t tempoUltimaInterrupcaoWifi  = 0;

//Headers
void calculateAndSendTelemetryFromPluviometer(int delayMsg);
void getAndSendTelemetryFromBMPSensor(int delayMsg, ThingsBoardWrapper *clientMQTT);
void getAndSendTelemetryFromSISensor(int delayMsg, ThingsBoardWrapper *clientMQTT);
void resetSettings();

RTC_DATA_ATTR int resetInvokeCounter;  // quantidade de vezes que a balança do pluviometro girou
RTC_DATA_ATTR int rainCounter1;
RTC_DATA_ATTR int notJustUpdated;

void IRAM_ATTR contarQtdInterrupcoesAnemometro();
void IRAM_ATTR incrementPluvi();
void IRAM_ATTR resetWifiManagerIfRequested();
void processSharedAttributeUpdate(const char* topic, byte* payload, unsigned int length);
void wait(int s, boolean show_seconds);
void setupDeepSleep();

// Para configurar seções críticas (interrupções de ativação e interrupções de desativação não disponíveis)
// usado para desabilitar e interromper interrupções (cuida da sincronização entre codigo principal e interrupção)
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void setup() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  Serial.println("----- Acordei ^_º -----");
  setupDeepSleep();
  pluvi.setupInterruptionHandler(incrementPluvi);
  anemometro.setupInterruptionHandler(contarQtdInterrupcoesAnemometro);
  wifiControl.setupInterruptionHandler(resetWifiManagerIfRequested);
  //wifiControl.setProcessSharedAttHandler(processSharedAttributeUpdate);
  Serial.printf("Rain counter %d.\n", rainCounter1);
  if (wifiControl.connectWifi()) {  // conexão com wi-fi
    clientMQTT.mqttServerURL = wifiControl.mqttServerURL;
    clientMQTT.mqttDeviceToken = wifiControl.mqttDeviceToken;
    if (clientMQTT.connectToIoTServer()) {
      int delayMsg = 5; // 5 segundos
      bool debug   = false;
      calculateAndSendTelemetryFromPluviometer(delayMsg);
      clientMQTT.sendTelemetryFloat("winSpeed", anemometro.getWinSpeed(), delayMsg, debug);
      clientMQTT.sendTelemetryString("winPointer", anemoscopio.getDirectionAsCardialPoint().c_str(), delayMsg, debug);
      clientMQTT.sendTelemetryFloat("battery", bateria.getPercentageBatteryValue(), delayMsg, debug);
      clientMQTT.sendTelemetryFloat("battery_2", bateria.calc_battery_percentage(), delayMsg, debug);
      clientMQTT.sendTelemetryInt("battery_raw", bateria.getRawBatteryValue(), delayMsg, debug);
      clientMQTT.sendTelemetryInt("battery_voltage", bateria.getVoltage(), delayMsg, debug);
      getAndSendTelemetryFromBMPSensor(delayMsg, &clientMQTT);
      getAndSendTelemetryFromSISensor(delayMsg, &clientMQTT);

      clientMQTT.firmwareUpdate(notJustUpdated);
      notJustUpdated = 1;
    }

    clientMQTT_elton.mqttServerURL = "https://thingsboard.ebt.synology.me";
    clientMQTT_elton.mqttDeviceToken = wifiControl.mqttDeviceToken;
    Serial.println("Enviando para o thingsboard bkp: 'thingsboard.ebt.synology.me'");
    if (clientMQTT_elton.connectToIoTServer()) {
      int delayMsg = 5; // 5 segundos
      bool debug   = false;
      calculateAndSendTelemetryFromPluviometer(delayMsg);
      clientMQTT_elton.sendTelemetryFloat("winSpeed", anemometro.getWinSpeed(), delayMsg, debug);
      clientMQTT_elton.sendTelemetryString("winPointer", anemoscopio.getDirectionAsCardialPoint().c_str(), delayMsg, debug);
      clientMQTT_elton.sendTelemetryFloat("battery", bateria.getPercentageBatteryValue(), delayMsg, debug);
      clientMQTT_elton.sendTelemetryFloat("battery_2", bateria.calc_battery_percentage(), delayMsg, debug);
      clientMQTT_elton.sendTelemetryInt("battery_raw", bateria.getRawBatteryValue(), delayMsg, debug);
      clientMQTT_elton.sendTelemetryInt("battery_voltage", bateria.getVoltage(), delayMsg, debug);
      getAndSendTelemetryFromBMPSensor(delayMsg, &clientMQTT_elton);
      getAndSendTelemetryFromSISensor(delayMsg, &clientMQTT_elton);

      clientMQTT_elton.firmwareUpdate(notJustUpdated);
      notJustUpdated = 1;
    }
  } else {
    Serial.println("Unable to connect Wifi!");
  }
  Serial.print("resetInvokeCounter:");
  Serial.println(resetInvokeCounter);
  if (resetInvokeCounter > 5) {
    resetSettings();
  }
  Serial.print(" ----- Vou dormir -_- -----");
  Serial.flush();
  esp_wifi_stop();
  esp_deep_sleep_start();  //Iniciar Deep Sleep
  REG_SET_FIELD(RTC_CNTL_REG, RTC_CNTL_DBIAS_WAK, 4);
  REG_SET_FIELD(RTC_CNTL_REG, RTC_CNTL_DBIAS_SLP, 4);
}



void loop() {
}

void setupDeepSleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);  // tempo deepSleep
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause == ESP_SLEEP_WAKEUP_EXT0) {  // incremento pluviometro
    portENTER_CRITICAL_ISR(&mux);        // início da seção crítica
    rainCounter1++;
    portEXIT_CRITICAL_ISR(&mux);  // fim da seção crítica
  }
  if (cause == ESP_SLEEP_WAKEUP_EXT1) {  // reset wificonfigs
    Serial.print("resetInvokeCounter:");
    Serial.println(resetInvokeCounter);
    if (resetInvokeCounter > 5) {
      resetSettings();
    }
  }
}

void calculateAndSendTelemetryFromPluviometer(int delayMsg) {
  int localRainCouter = rainCounter1;
  Serial.printf("Contador de interrupções: %d.\n", localRainCouter);
  //if (localRainCouter > 0) {
    bool sent = clientMQTT.sendTelemetryInt("pluviometer", localRainCouter, delayMsg);
    if (sent) {
      portENTER_CRITICAL_ISR(&mux);     // início da seção crítica
      rainCounter1 -= localRainCouter;  //
      portEXIT_CRITICAL_ISR(&mux);      // fim da seção crítica
    } else {
      Serial.println("Pulsos do pluvi não enviados!");
    }
  //}
}

float readAltitude(float seaLevelhPa, float pressure) {
  float altitude;
  altitude = 44330 * (1.0 - pow(pressure / seaLevelhPa, 0.1903));
  return altitude;
}

float getPressure() {
  return bmpSensor.readPressure() * 0.01063;  // /100 * 1,0
}

void getAndSendTelemetryFromBMPSensor(int delayMsg, ThingsBoardWrapper *clientMQTT) {
  if (!bmpSensor.begin(0x76)) {
    Serial.println("Falha ao tentar ler o sensor BMP280");
  } else {
    float pressure = getPressure();
    float altitude = readAltitude(1020, pressure);
    float temperature = bmpSensor.readTemperature();
    Serial.printf("BMP infos temp: %.2f, pressure: %.2f, altitude: %.2f.\n", temperature, pressure, altitude);
    clientMQTT->sendTelemetryFloat("temperature", temperature, delayMsg);
    clientMQTT->sendTelemetryFloat("pressure", pressure, delayMsg);
    clientMQTT->sendTelemetryFloat("altitude", altitude, delayMsg);
  }
}

void getAndSendTelemetryFromSISensor(int delayMsg, ThingsBoardWrapper *clientMQTT) {
  if (!siSensor.begin()) {
    //envio de dados do sensor SI7021
    Serial.println("Falha ao tentar ler o sensor SI7021");
  } else {
    float humidity = siSensor.readHumidity();
    Serial.printf("SI7021 infos humidity: %.2f.\n", humidity);
    clientMQTT->sendTelemetryFloat("humidity", siSensor.readHumidity(), delayMsg);
    //wifiControl.sendTelemetryFloat("SItemperature", siSensor.readTemperature());
  }
}



void IRAM_ATTR contarQtdInterrupcoesAnemometro() {
  //faz o debounce do reed switch
  if (xTaskGetTickCount() - tempoUltimaInterrupcaoAnem > 200) {
    anemometro.somarPulsos(1);
    tempoUltimaInterrupcaoAnem = xTaskGetTickCount();
  }
}

void IRAM_ATTR incrementPluvi() {
  if (xTaskGetTickCount() - tempoUltimaInterrupcaoPluvi > 200) {
    portENTER_CRITICAL_ISR(&mux);  // início da seção crítica
    rainCounter1++;
    //versão do millis () que funciona a partir da interrupção
    tempoUltimaInterrupcaoPluvi = xTaskGetTickCount();
    portEXIT_CRITICAL_ISR(&mux);  // fim da seção crítica
  }
}

void IRAM_ATTR resetWifiManagerIfRequested() {
  //Apaga os dados da rede wifi gravados na memoria e reinicia o ESP
  if (xTaskGetTickCount() - tempoUltimaInterrupcaoWifi > 200) {
    resetInvokeCounter++;
    tempoUltimaInterrupcaoWifi = xTaskGetTickCount();
  }
}

void resetSettings() {  //reeconexão em caso de queda
  Serial.printf("Resetando configurações do Wifi!\n");
  resetInvokeCounter = 0;
  wifiControl.resetSettings();
  ESP.restart();
}


