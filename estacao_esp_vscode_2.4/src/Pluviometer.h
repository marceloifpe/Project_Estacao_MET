/**
 * Classe Pluviometer
 * * */
#ifndef Pluviometer_H
#define Pluviometer_H

#define Bucket_Size_EU 0.364      // tamanho do bucket do pluviometro ml
//Cálculo em mm = volume_bucket / área do pluviometro
//configuração atual 7,5ml / 56,7cm = 0,132
// nova 18,2ml / 9.1raio = 0,364mm

RTC_DATA_ATTR int rainCounter;  // quantidade de vezes que a balança do pluviometro girou
static int debounce = 250;

class Pluviometer {

private:
  gpio_num_t gpioRainSensorPin = GPIO_NUM_26;  // Rain REED-ILS sensor GPIO 26 on ESP32
  inline void setup();

public:
  Pluviometer(gpio_num_t gpioRainSensorPin);
  inline void increaseRainCounter();
  inline float getRainCounterValue();
  inline void setRainCounterValue(float value);
  inline void clearRainCounterValue();
  inline float getRainVolumeInMM(void);
  inline void static forceIncrement();
  void setupInterruptionHandler(void (*func)());
};

Pluviometer::Pluviometer(gpio_num_t sGpioRainSensorPin) {
  gpioRainSensorPin = sGpioRainSensorPin;
  setup();
}

inline void Pluviometer::setup() {
  // ALTERAÇÃO 1: Muda para INPUT_PULLUP (mantém o pino em 3.3V internamente)
  pinMode(gpioRainSensorPin, INPUT_PULLUP);

  // ALTERAÇÃO 2: Muda o wakeup para '0' (acorda quando o sinal vai para GND/LOW)
  esp_sleep_enable_ext0_wakeup(gpioRainSensorPin, 0);
}

inline void Pluviometer::setupInterruptionHandler(void (*func)()) {
  // ALTERAÇÃO 3: Muda para FALLING (detecta a descida de 3.3V para 0V quando o ímã passa)
  attachInterrupt(digitalPinToInterrupt(gpioRainSensorPin), func, FALLING);
}

inline void Pluviometer::increaseRainCounter() {
  rainCounter++;
}

inline float Pluviometer::getRainCounterValue() {
  return rainCounter;
}

inline void Pluviometer::setRainCounterValue(float value) {
  rainCounter = value;
}

inline void Pluviometer::clearRainCounterValue() {
  setRainCounterValue(0);
}

#endif