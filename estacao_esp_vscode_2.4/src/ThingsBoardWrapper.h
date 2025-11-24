/**
 * Classe ThingsBoardWrapper
 * 
 * 
 */
#ifndef ThingsBoardWrapper_H
#define ThingsBoardWrapper_H
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
//#include <ThingsBoard.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


#define ENCRYPTED false
constexpr char SHARED_REQUEST_KEY[] = "sharedKeys";
constexpr char SHARED_RESPONSE_KEY[] = "shared";
constexpr char ATTRIBUTE_TOPIC[] = "v1/devices/me/attributes";
constexpr char ATTRIBUTE_REQUEST_TOPIC[] = "v1/devices/me/attributes/request/1";
constexpr char ATTRIBUTE_RESPONSE_SUBSCRIBE_TOPIC[] = "v1/devices/me/attributes/response/+";
constexpr char ATTRIBUTE_RESPONSE_TOPIC[] = "v1/devices/me/attributes/response/1";
constexpr char TELEMETRY_TOPIC[] = "v1/devices/me/telemetry";
constexpr char CURR_FW_TITLE_KEY[] = "current_fw_title";
constexpr char CURR_FW_VER_KEY[] = "current_fw_version";
constexpr char FW_ERROR_KEY[] = "fw_error";
constexpr char FW_STATE_KEY[] = "fw_state";
constexpr char FW_STATE_DOWNLOADING[] = "DOWNLOADING";
constexpr char FW_STATE_DOWNLOADED[] = "DOWNLOADED";
constexpr char FW_STATE_VERIFIED[] = "VERIFIED";
constexpr char FW_STATE_UPDATING[] = "UPDATING";
constexpr char FW_STATE_UPDATED[] = "UPDATED";
constexpr char FW_STATE_FAILED[] = "FAILED";
constexpr char FW_VER_KEY[] = "fw_version";
constexpr char FW_TITLE_KEY[] = "fw_title";
constexpr char FW_CHKS_KEY[] = "fw_checksum";
constexpr char FW_CHKS_ALGO_KEY[] = "fw_checksum_algorithm";
constexpr char FW_SIZE_KEY[] = "fw_size";


WiFiClient cliente;  // objetos WiFi e ThingsBoardWrapper
PubSubClient tb(cliente);
DynamicJsonDocument sharedAttSubscribesDoc(1024);

class ThingsBoardWrapper {

private:
  String fwTitle;
  String fwVersion;
  // Statuses for updating
  bool currentFWSent = false;
  bool updateRequestSent = false;
  String currFirmTitle = "est_met_firm";
  String currFirmVersion = "2.2.3";
  String clientId = "est_met";

public:
  String mqttServerURL = "15.229.47.249";
  int mqttServerPort = 1883;
  String mqttDeviceToken = "estacaoteste";
  ThingsBoardWrapper(String currFirmTitle, String currFirmVersion, String clientId);
  ThingsBoardWrapper(String currFirmTitle, String currFirmVersion, String clientId, String mqttServerURL, int mqttServerPort, String mqttDeviceToken);
  void setup();
  bool connectToIoTServer();
  bool sendTelemetryFloat(const char *key, float value, int delay = 0, bool debug = false);
  bool sendTelemetryInt(const char *key, int value, int delay = 0, bool debug = false);
  bool sendTelemetryString(const char *key, const char *value, int delay = 0, bool debug = false);
  bool sendTelemetryJson(const JsonObject doc, int delay = 0, bool debug = false);
  bool subscribeAttribute(const char *key, int delay = 0, bool debug = false);
  bool subscribe();
  bool sendAttributeRequestJson(const char *attributeKey, int delay = 0, bool debug = false);
  void sendTestTelemetry();
  void firmwareUpdate(int notJustUpdated);
  bool sendFirmInfos();
  JsonObject getFirmwareInfo();
  void wait(int s, boolean show_seconds);


  //RECEBE DADOS DO SERVIDOR MQTT
  static void processSharedAttributeUpdate(const char *topic, byte *payload, unsigned int length) {
    DynamicJsonDocument doc(512);
    deserializeJson(doc, payload, length);
    JsonObject root = doc.as<JsonObject>();
    String output;

    if (strcmp(topic, ATTRIBUTE_TOPIC) == 0) {
      for (JsonObject::iterator it = root.begin(); it != root.end(); ++it) {
        const char *key = it->key().c_str();
        Serial.println(key);
        Serial.println(it->value().as<const char *>());
        if (sharedAttSubscribesDoc.containsKey(key)) {
          const char *value = "a";
          sharedAttSubscribesDoc[it->key()];
          Serial.printf("Tópico %s!", key);
        }
      }
    } else {
      if (strcmp(topic, ATTRIBUTE_RESPONSE_TOPIC) == 0) {
        if (root.containsKey(SHARED_RESPONSE_KEY)) {
          JsonObject sharedDoc = root[SHARED_RESPONSE_KEY];
          for (JsonObject::iterator it = sharedDoc.begin(); it != sharedDoc.end(); ++it) {
            const char *key = it->key().c_str();
            Serial.println(key);
            Serial.println(it->value().as<const char *>());
            //if (sharedAttSubscribesDoc.containsKey(key)) {
              //sharedAttSubscribesDoc[key](it->value().as<const char *>());
              //Serial.printf("Tópico %s!", key);
            //}
            std::string strKey(key);

            if (strKey.compare("acionador1") == 1) {
              Serial.println("Acionando a bomba: ");              
              int value = it->value().as<int>();
              Serial.println(value);
              pinMode(26, OUTPUT);
              digitalWrite(26, value); 

            }
          }
        }
      } else {
        Serial.printf("Tópico não tratado:%s!", topic);
      }
    }
  }
};

ThingsBoardWrapper::ThingsBoardWrapper(String sCurrFirmTitle, String sCurrFirmVersion, String sClientId) {
  currFirmTitle = sCurrFirmTitle;
  currFirmVersion = sCurrFirmVersion;
  clientId = sClientId;
  setup();
}

ThingsBoardWrapper::ThingsBoardWrapper(String sCurrFirmTitle, String sCurrFirmVersion, String sClientId,
                                       String mqttServerURL, int mqttServerPort, String mqttDeviceToken) {
  currFirmTitle = sCurrFirmTitle;
  currFirmVersion = sCurrFirmVersion;
  clientId = sClientId;
  mqttServerURL = mqttServerURL;
  mqttServerPort = mqttServerPort;
  mqttDeviceToken = mqttDeviceToken;
  setup();
}

void ThingsBoardWrapper::setup() {
  tb.setCallback(processSharedAttributeUpdate);
}

bool ThingsBoardWrapper::connectToIoTServer() {
  // Connect to WiFi network
  if (tb.connected()) {
    return true;
  } else {
    //Serial.printf("Conectando a: %s...\n", mqttServerURL.c_str());
    tb.setServer(mqttServerURL.c_str(), mqttServerPort);
    String clientId = "est_met_-";
    clientId += String(random(0xffff), HEX);
    if (tb.connect(clientId.c_str(), mqttDeviceToken.c_str(), NULL)) {
      subscribe();
      subscribeAttribute("acionador1");
      subscribeAttribute("acionador2");
      return true;
    } else {
      Serial.printf("Falha ao conectar em %s...\n", mqttServerURL.c_str());
      return false;
    }
  }
}

void ThingsBoardWrapper::firmwareUpdate(int notJustUpdated) {
  Serial.printf("Verificando se há atualizações em %s:%d..\n", mqttServerURL.c_str(), 8080);
  Serial.printf("Firmware atual: %s %s!\n", currFirmTitle.c_str(), currFirmVersion.c_str());
  JsonObject firmwareInfosJson = getFirmwareInfo();
  if (firmwareInfosJson.containsKey(FW_VER_KEY) && firmwareInfosJson.containsKey(FW_TITLE_KEY)) {
    const char *fwTitle_c = firmwareInfosJson[FW_TITLE_KEY];
    const char *fwVersion_c = firmwareInfosJson[FW_VER_KEY];
    std::string fwTitle(fwTitle_c);
    std::string fwVersion(fwVersion_c);

    if (fwVersion.compare(currFirmVersion.c_str()) != 0 || fwTitle.compare(currFirmTitle.c_str()) != 0) {
      Serial.printf("Novo Firmware disponível: %s %s!\n", fwTitle.c_str(), fwVersion.c_str());
      Serial.println("Iniciando atualização:");
      Serial.println("\tBaixando firmware...");
      sendTelemetryString(FW_STATE_KEY, "DOWNLOADING");
      char URL[150];
      sprintf(URL, "http://%s:%d/api/v1/%s/firmware?title=%s&&version=%s", mqttServerURL.c_str(), 8080, mqttDeviceToken.c_str(), fwTitle.c_str(), fwVersion.c_str());
      delay(1);
      Serial.println(URL);
      Serial.println("\tAtualizando...");
      sendTelemetryString(FW_STATE_KEY, "UPDATING");
      t_httpUpdate_return httpUpdateReturn = httpUpdate.update(cliente, URL);
      switch (httpUpdateReturn) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
          sendTelemetryString(FW_STATE_KEY, "FAILED");
          sendTelemetryString("fw_error", httpUpdate.getLastErrorString().c_str());
          break;

        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;

        case HTTP_UPDATE_OK:
          Serial.println("HTTP_UPDATE_OK");
          sendFirmInfos();
          break;
      }
    } else {
      if (notJustUpdated == 0) {
        sendFirmInfos();
        Serial.println("O dispositivo acabou de ser atualizado!");
      } else {
        Serial.println("O dispositivo já está atualizado!");
      }
    }
  } else {
    Serial.println("Não há atualizações definidas para o dispositivo!");
  }
}

bool ThingsBoardWrapper::sendFirmInfos() {
  StaticJsonDocument<512> doc;
  doc[CURR_FW_TITLE_KEY] = currFirmTitle;
  doc[CURR_FW_VER_KEY] = currFirmVersion;
  doc[FW_STATE_KEY] = FW_STATE_UPDATED;
  tb.disconnect();  // O envio das informações referentes à atualização devem ser as primeiras da conexão.
  return sendTelemetryJson(doc.as<JsonObject>(), 5, true);
}

bool ThingsBoardWrapper::subscribeAttribute(const char *key, int delay, bool debug) {
  //sharedAttSubscribesDoc[key] = func;
  return sendAttributeRequestJson(key, delay, debug);
}

bool ThingsBoardWrapper::subscribe() {
  return tb.subscribe(ATTRIBUTE_TOPIC);
}

bool ThingsBoardWrapper::sendTelemetryFloat(const char *key, float value, int delay, bool debug) {
  StaticJsonDocument<256> doc;
  doc[key] = value;
  return sendTelemetryJson(doc.as<JsonObject>(), delay, debug);
}

bool ThingsBoardWrapper::sendTelemetryInt(const char *key, int value, int delay, bool debug) {
  StaticJsonDocument<256> doc;
  doc[key] = value;
  return sendTelemetryJson(doc.as<JsonObject>(), delay, debug);
}

bool ThingsBoardWrapper::sendTelemetryString(const char *key, const char *value, int delay, bool debug) {
  StaticJsonDocument<256> doc;
  doc[key] = value;
  return sendTelemetryJson(doc.as<JsonObject>(), delay, debug);
}

bool ThingsBoardWrapper::sendTelemetryJson(const JsonObject doc, int delay, bool debug) {
  wait(delay, true);
  char buffer[256];
  serializeJson(doc, buffer);
  bool sent = false;
  if (connectToIoTServer()) {
    sent = tb.publish(TELEMETRY_TOPIC, buffer);
    if (debug) {
      if (sent) {
        Serial.println("Mensagem enviada com sucesso!");
      } else {
        Serial.println("Mensagem não enviada!");
      }
    }
  } else {
    Serial.println("Mensagem não enviada: não foi possível conectar ao servidor!");
  }

  return sent;
}
bool ThingsBoardWrapper::sendAttributeRequestJson(const char *attributeKey, int delay, bool debug) {
  wait(delay, true);
  char request[100];
  sprintf(request, "{'sharedKeys': '%s'}", attributeKey);
  bool sent = false;
  if (connectToIoTServer()) {
    sent = tb.publish(ATTRIBUTE_REQUEST_TOPIC, request);
    if (debug) {
      if (sent) {
        Serial.println("Requisição de atributos enviada com sucesso!");
      } else {
        Serial.println("Requisição de atributos não enviada!");
      }
    }
  } else {
    Serial.println("Requisição de atributos não enviada: não foi possível conectar ao servidor!");
  }

  return sent;
}


JsonObject ThingsBoardWrapper::getFirmwareInfo() {
  char URL[100];
  sprintf(URL, "http://%s:%d/api/v1/%s/attributes", mqttServerURL.c_str(), 8080, mqttDeviceToken.c_str());
  HTTPClient http;
  http.useHTTP10(true);
  http.begin(cliente, URL);
  http.GET();
  String resp = http.getString();
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, resp);
  JsonObject object = doc.as<JsonObject>();
  http.end();
  return object[SHARED_RESPONSE_KEY];
}

void ThingsBoardWrapper::wait(int s, boolean show_seconds) {
  long dataUltimaAfericao = millis();
  for (int i = 0; i < s; i++) {
    //Exibir segundos
    if (show_seconds && i % 5 == 0) {
      Serial.print("(");
      Serial.print(i);
      Serial.print("s)");
    }
    Serial.print(".");
    dataUltimaAfericao = millis();
    while (millis() - dataUltimaAfericao < 1000) {
      tb.loop();
    }
  }
  Serial.println("");
}

#endif