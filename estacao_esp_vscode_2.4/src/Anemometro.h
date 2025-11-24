#include "esp32-hal.h"
/**
 * Classe Anemometro
 * 
 * Calcula a velocidade do vento com base em interrupções aferidas
 * em um intervalo de 30 segundos.
 * 
 */

 RTC_DATA_ATTR int numPulsosAnemometro = 0;

class Anemometro {

  // -----------------------------------------------------------------

  const float pi = 3.14159265;  // Número de pi

private:
  int pin = 25;
  int periodoAfericao = 5000;            // (Default) Período Aferição Vento em milésimos de segundo (ms)
  int raio = 65;                          // (Default) Raio do anemometro em milímetros (mm)
  //volatile byte numPulsosAnemometro = 0;  // Contador para o sensor Si7021 reed switch no anemômetro 

  // -----------------------------------------------------------------
  void setup();
  /*
  * Função de cálculo do RPM
  * 
  * Função deve calcular o rpm do anemômetro
  * 
  * Não contém parâmetros
  * 
  * @return void
  */
  int calcularRpm();

  /*
  * Função de cálculo da velocidade do vento em Km/h
  * 
  * Função deve calcular a velocidade do vento daquele momento do anemômetro
  * 
  * Não contém parâmetros
  * 
  * @return void
  */
  float calcularVelocidadeVento(int rpm);

  /*
   * Exibe as informações aferidas:
   *  - Número de pulsos do anemômetro
   *  - Rotações por minuto(RPM)
   *  - Velocidade do vento KM/h
   */
  void printInfos(int rpm, float velocidadeVento);

public:
  /**
   * Construtor da Classe Anemometro
   * 
   * Ao ser chamado, o construtor vazio,
   * os atributos pin, o periodoAfericao e o raio 
   * teram valores atribuidos com dados pré determinados/setados.
   * 
   * return void
   */
  Anemometro();

  /**
   * Construtor da Classe Anemometro
   * 
   * @param int sPin, define o pino de aferição
   * 
   * @return void
   */
  Anemometro(int sPin);

  /**
   * Construtor da Classe Anemometro
   * 
   * @param int sPin, define o pino de aferição
   * @param int sPeriodoAfericao, define o período entre aferições
   * @param int sRaio, define o raio do anemometro
   * 
   * @return void
   */
  Anemometro(int sPin, int sPeriodoAfericao, int sRaio);

  /*
  * Função de soma de quantidade de Pulsos do anemômetro
  * 
  * Função deve calcular a quantidade de pulsos do anemômetro, somando mais um a cada pulso
  * 
  * @param int quant, deve receber a quantidade de pulsos
  * 
  * @return void
  */
  void somarPulsos(int quant);

  /*
  * Função para resetar contador
  * 
  * Função deve resetar um contador do anemômetro se a velocidade do vento já tenha sido aferida
  * 
  * Não contém parâmetros
  * 
  * @return void
  */
  void resetarContador();

  /*
  * Função para pegar a velocidade do vento
  * 
  * Função deve retornar o resultado do cálculo da velocidade do vento em Km/h
  * 
  * Não contém parâmetros
  * 
  * @return float
  */
  float getWinSpeed();

  /*
  * Função para setar o pin do anemômetro
  * 
  * Não contém parâmetros
  * 
  * @return void
  */
  void setPin(int sPin);

  /*
  * Função irá retornar o pino de conexão do anemometro
  * 
  * Não contém parâmetros
  * 
  * @return pin
  */
  int getPin();
  /*
  * Função para setar o Período de Aferição
  * 
  * Função deve setar o valor do período de aferição
  * 
  * @params int sPeriodoAfericao
  * 
  * @return void
  */
  void setPeriodoAfericao(int sPeriodoAfericao);

  /*
   * Função que retorna o perído de aferição setado
   * 
   * @return int 
   */
  int getPeriodoAfericao();

  /*
   * Função para setar o raio do anemômetro
   * @param int sRaio, raio em milimetros
   * 
   * @return void
   */
  void setRaio(int sRaio);

  /*
   * Função para retornar o raio do anemômetro em milimetros
   * 
   * Não contém parâmetros
   * 
   * @return int
   */
  int getRaio();

  void setupInterruptionHandler(void (*func)());
};

Anemometro::Anemometro() {
  setup();
}

Anemometro::Anemometro(int sPin) {
  pin = sPin;
  setup();
}

Anemometro::Anemometro(int sPin, int sPeriodoAfericao, int sRaio) {
  pin = sPin;
  periodoAfericao = sPeriodoAfericao;
  raio = sRaio;
  setup();
}

void Anemometro::setup() {
  pinMode(pin, INPUT_PULLDOWN);
}

void Anemometro::setupInterruptionHandler(void (*func)()) {
  attachInterrupt(digitalPinToInterrupt(pin), func, RISING);
}

int Anemometro::calcularRpm() {
  return ((numPulsosAnemometro)*60) / (periodoAfericao / 1000);
}

float Anemometro::calcularVelocidadeVento(int rpm) {
  return (((2 * pi * raio * rpm) / 60) / 1000) * 3.6;
}

void Anemometro::somarPulsos(int qtt) {
  numPulsosAnemometro += qtt;
}

void Anemometro::resetarContador() {
  numPulsosAnemometro = 0;
}

float Anemometro::getWinSpeed() {
  // Aguardando aferição da velocidade do vento;
  Serial.println("Aferindo velocidade do vento!");
  unsigned long dataUltimaAfericao = millis();
  while (millis() < dataUltimaAfericao + periodoAfericao)
    ;
  int rpm = calcularRpm();
  float windSpeed = calcularVelocidadeVento(rpm);
  printInfos(rpm, windSpeed);  //Exibir aferição
  resetarContador();
  return windSpeed;
}

void Anemometro::setPin(int sPin) {
  pin = sPin;
}

int Anemometro::getPin() {
  return pin;
}

void Anemometro::setPeriodoAfericao(int sPeriodoAfericao) {
  periodoAfericao = sPeriodoAfericao;
}

int Anemometro::getPeriodoAfericao() {
  return periodoAfericao;
}

void Anemometro::setRaio(int sRaio) {
  raio = sRaio;
}

int Anemometro::getRaio() {
  return raio;
}

void Anemometro::printInfos(int rpm, float velocidadeVento) {
  Serial.printf("Pulsos anemômetro: %d; RPM: %d; Vel. Vento: %.2f [km/h].\n", numPulsosAnemometro, rpm, velocidadeVento);
}
