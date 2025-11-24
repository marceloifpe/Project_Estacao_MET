/**
 * Classe Anemoscopio
 * 
 * 
 */

class Anemoscopio {
private:
  const String compass[17] = {  //Pontos cardiais, colaterais e subcolaterais
    "N  ", "NNE", "NE ", "NEE",
    "E  ", "SEE", "SE ", "SSE",
    "S  ", "SSW", "SW ", "SWW",
    "W  ", "NWW", "NW ", "NNW", "???"
  };
  unsigned long tempoUltimaInterrupcao = 0;  //Data da ultima interrupção
  int debounce = 250;                        //Tempo de debouncing
  //int pins[8] = { 21, 4, 36, 27, 33,       // new version pcb
  //                15, 32, 14 };
  int pins[8] = { 4, 21, 13, 27, 33,         //Pins do ESP32
                  15, 32, 14 };
  void setup();

public:
  Anemoscopio();
  Anemoscopio(const int pins[]);
  byte getDirectionBin();
  int getDirectionAsInt();
  String getDirectionAsCardialPoint();
  int* getpins();
  void setDebounce(int sDebounce);
  int getDebounce();
};

Anemoscopio::Anemoscopio() {
  setup();
}

Anemoscopio::Anemoscopio(const int newPins[]) {
  for (int i = 0; i < 8; i++)
    pins[i] = newPins[i];
  setup();
}

void Anemoscopio::setup() {
  for (int i = 7; i >= 0; i--) {
    pinMode(pins[i], INPUT_PULLDOWN);
  }
}

byte Anemoscopio::getDirectionBin() {
  unsigned long tempoInterrupcao = millis();
  while (tempoInterrupcao - tempoUltimaInterrupcao < debounce)
    ;
  //Caso Default
  byte sDirect = 0;
  //Varre as 8 posições
  for (int i = 7; i >= 0; i--) {
    //Serial.print(i);
    //Incrementa no byte
    sDirect = sDirect | (digitalRead(pins[7 - i]) << i);
    //sDirect = sDirect | ( convert(digitalRead(pins[7-i])) << i);
  }
  //Atualiza a variável de debounce
  tempoUltimaInterrupcao = tempoInterrupcao;
  return sDirect;
}

int Anemoscopio::getDirectionAsInt() {
  byte bDirection = getDirectionBin();
  int pointer = -1;
  int direction = bDirection;
  switch (direction) {
    // ---- Casos N, NNE, NE, NEE ----
    case 1:
      pointer = 0;
      break;
    case 3:
      pointer = 1;
      break;
    case 2:
      pointer = 2;
      break;
    case 6:
      pointer = 3;
      break;

    // ---- Casos E, SEE, SE, SSE ----
    case 4:
      pointer = 4;
      break;
    case 12:
      pointer = 5;
      break;
    case 8:
      pointer = 6;
      break;
    case 24:
      pointer = 7;
      break;

    // ---- Casos S, SSW, SW, SWW ----
    case 16:
      pointer = 8;
      break;
    case 48:
      pointer = 9;
      break;
    case 32:
      pointer = 10;
      break;
    case 96:
      pointer = 11;
      break;

    // ---- Casos W, NWW, NW, NNW ----
    case 64:
      pointer = 12;
      break;
    case 192:
      pointer = 13;
      break;
    case 128:
      pointer = 14;
      break;
    case 129:
      pointer = 15;
      break;

    //---- Nenhum dos Casos ----
    default:
      pointer = 16;
      // if nothing else matches, do the default
      // default 16, "???" mainly for debugging
      break;
  }

  //Exibir informações
  //toString();
  return pointer;
}

String Anemoscopio::getDirectionAsCardialPoint() {
  int direction = getDirectionAsInt();
  if (direction > -1 && direction < 16) {
    String str = compass[direction];
    char* cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    return cstr;
  } else {
    return "ERR";
  }
}

//Retorna o vetor de pins na memória
int* Anemoscopio::getpins() {
  return pins;
}

//Seta o tempo de espera entre as aferições
void Anemoscopio::setDebounce(int sDebounce) {
  debounce = sDebounce;
}

//Retorna o tempo de espera entre as aferições
int Anemoscopio::getDebounce() {
  return debounce;
}
