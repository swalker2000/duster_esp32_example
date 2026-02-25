# Пример IOT клиента работающего с сервисом гарантии доставки работающим поверх MQTT
## https://github.com/swalker2000/duster_broker
Пример consumer на основе esp32 : https://github.com/swalker2000/duster_esp32_example
Для компиляции необходимо в основной директории скетча создать файл Secret.h со следующим содержимым: 
```c++
#define SSID          "MY_SSID"
#define WIFI_PASS     "MY_WIFI_PASS"
#define URL           "MQTT_URL"
#define PORT          8883
#define MQTT_USERNAME "MQTT_USERNAME"
#define MQTT_PASS     "MQTT_PASSWORD"
```