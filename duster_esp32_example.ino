#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "Secret.h"

// ================= НАСТРОЙКИ =================
const char ssid[]     = SSID;
const char password[] = WIFI_PASS;

const char mqtt_broker[]   = URL;
const int  mqtt_port       = PORT;                 // для SSL
const char mqtt_username[] = MQTT_USERNAME;
const char mqtt_password[] = MQTT_PASS;

const char deviceId[] = "device123";                // идентификатор устройства

// ================= ГЛОБАЛЬНЫЕ ОБЪЕКТЫ =================
WiFiClientSecure net;
PubSubClient client(net);

// ================= ФУНКЦИИ ПОДКЛЮЧЕНИЯ =================
void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");
}

void connectToMQTT() {
  Serial.print("Connecting to MQTT broker");

  net.setInsecure();   // отключаем проверку сертификата (для теста)

  client.setServer(mqtt_broker, mqtt_port);
  client.setBufferSize(2048);              // увеличиваем буфер до 2048 байт
  client.setCallback(messageReceived);

  while (!client.connected()) {
    Serial.print(".");
    if (client.connect(deviceId, mqtt_username, mqtt_password)) {
      Serial.println("\nMQTT connected");
      String subscribeTopic = "consumer/request/" + String(deviceId);
      client.subscribe(subscribeTopic.c_str());
      Serial.println("Subscribed to: " + subscribeTopic);
    } else {
      delay(1000);
    }
  }
}

// ================= ОБРАБОТЧИК ВХОДЯЩИХ СООБЩЕНИЙ =================
void messageReceived(char* topic, byte* payload, unsigned int length) {
  // Преобразуем топик и payload в String для удобства
  String topicStr = String(topic);
  String payloadStr;
  for (unsigned int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  Serial.println("Message received:");
  Serial.println("Topic: " + topicStr);
  Serial.println("Payload length: " + String(payloadStr.length()));
  Serial.println("Payload: " + payloadStr);

  // Проверяем, что топик предназначен этому устройству
  String expectedPrefix = "consumer/request/";
  if (!topicStr.startsWith(expectedPrefix)) return;

  String receivedDeviceId = topicStr.substring(expectedPrefix.length());
  if (receivedDeviceId != deviceId) {
    Serial.println("Device ID mismatch – ignoring");
    return;
  }

  // Парсинг JSON
  DynamicJsonDocument doc(1024);   // размер можно увеличить при необходимости
  DeserializationError error = deserializeJson(doc, payloadStr);
  if (error) {
    Serial.print("JSON parsing error: ");
    Serial.println(error.c_str());
    return;
  }

  long id = doc["id"];                     // id запроса
  const char* command = doc["command"];    // команда

  Serial.print("Extracted command: ");
  Serial.println(command);

  // --- Обработка команд ---
  if (strcmp(command, "digitalWrite") == 0) {
    // Получаем объект data
    JsonObject data = doc["data"];
    if (data.isNull()) {
      Serial.println("Error: data field missing for digitalWrite command");
    } else {
      int pinNumber = data["pinNumber"] | -1;   // если нет поля, будет -1
      bool pinValue = data["pinValue"] | false;
      if (pinNumber == -1) {
        Serial.println("Error: pinNumber missing or invalid");
      } else {
        pinMode(pinNumber, OUTPUT);
        digitalWrite(pinNumber, pinValue ? HIGH : LOW);
        Serial.printf("digitalWrite: pin %d set to %s\n", pinNumber, pinValue ? "HIGH" : "LOW");
      }
    }
  }
  else if (strcmp(command, "pinModeOutput") == 0) {
    // Получаем объект data
    JsonObject data = doc["data"];
    if (data.isNull()) {
      Serial.println("Error: data field missing for digitalWrite command");
    } else {
      int pinNumber = data["pinNumber"] | -1;   // если нет поля, будет -1
      if (pinNumber == -1) {
        Serial.println("Error: pinNumber missing or invalid");
      } else {
        pinMode(pinNumber, OUTPUT);
        //digitalWrite(pinNumber, pinValue ? HIGH : LOW);
        Serial.printf("pinMode: pin %d OUTPUT", pinNumber);
      }
    }
  }
  // При необходимости можно добавить другие команды
  // else if (strcmp(command, "anotherCommand") == 0) { ... }

  // --- Формирование и отправка ответа (всегда содержит только id) ---
  StaticJsonDocument<128> responseDoc;
  responseDoc["id"] = id;

  String responsePayload;
  serializeJson(responseDoc, responsePayload);

  String responseTopic = "consumer/response/" + String(deviceId);
  if (client.publish(responseTopic.c_str(), responsePayload.c_str())) {
    Serial.println("Response sent to " + responseTopic);
    Serial.println("Response payload: " + responsePayload);
  } else {
    Serial.println("Failed to send response");
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP32 MQTT JSON Example (PubSubClient + digitalWrite)");

  connectToWiFi();
  connectToMQTT();
}

// ================= LOOP =================
void loop() {
  client.loop();   // обязательно для поддержания связи и приёма сообщений

  // Если потеряли соединение с MQTT – переподключаемся
  if (!client.connected()) {
    Serial.println("MQTT disconnected, reconnecting...");
    connectToMQTT();
  }

  // Другие задачи...
}