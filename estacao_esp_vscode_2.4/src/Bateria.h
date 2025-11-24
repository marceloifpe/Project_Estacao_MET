/**
 * Classe Bateria
 * 
 * 
 */
#ifndef Bateria_H
#define Bateria_H

class Bateria {

  // ---------------- Atributos ----------------
private:
  float bateryLevel = 0.0;
  unsigned int raw = 0;
  int vbatPin = A13;                 //Pin Default
  float batteryCapacity = 2300.0f;  //Capacidade da Bateria
  void setup();
  // -------------------------------------------

public:
  Bateria();
  int getRawBatteryValue();
  float getPercentageBatteryValue();
  int getPin();
  uint32_t getVoltage();
  void setPin(int sPin);
  int getBatteryCapacity();
  void setBatteryCapacity(int sBatteryCapacity);
  float calc_battery_percentage();
  float mapf(float x, float in_min, float in_max, float out_min, float out_max);
  void toString();
};

Bateria::Bateria() {
  setup();
}

void Bateria::setup() {
  pinMode(vbatPin, INPUT);
}


int Bateria::getRawBatteryValue() {
  return analogRead(vbatPin);  // Deve-se multiplicar por 2 por causa do divisor de tensão.
}

float Bateria::getPercentageBatteryValue() {
  raw = getRawBatteryValue();
  float battery_percentage = mapf(raw, 1835.0f, batteryCapacity, 0, 100);
  Serial.printf("Battery raw: %d - %.2f\%\n", raw, battery_percentage);
  if (battery_percentage < 0)
    battery_percentage = 0;
  if (battery_percentage > 100)
    battery_percentage = 100;
  return battery_percentage;
}

float Bateria::calc_battery_percentage() {
  uint32_t battery_voltage_ui = analogReadMilliVolts(vbatPin);
  float battery_voltage = battery_voltage_ui * 0.002;    // we divided by 2, so multiply back
  float battery_percentage = mapf(battery_voltage, 3.20f, 4.1f, 0, 100);
  Serial.printf("Battery voltage: %.2f v - %.2f\%\n", battery_voltage, battery_percentage);
  if (battery_percentage < 0)
    battery_percentage = 0;
  if (battery_percentage > 100)
    battery_percentage = 100;
  return battery_percentage;
}

float Bateria::mapf(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint32_t Bateria::getVoltage() {
  return analogReadMilliVolts(vbatPin);
}


int Bateria::getPin() {
  return vbatPin;
}

void Bateria::setPin(int sPin) {
  vbatPin = sPin;
}

int Bateria::getBatteryCapacity() {
  return batteryCapacity;
}

void Bateria::setBatteryCapacity(int sBatteryCapacity) {
  batteryCapacity = sBatteryCapacity;
}
#endif