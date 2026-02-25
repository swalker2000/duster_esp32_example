# Example of an IoT client using a delivery guarantee service over MQTT
## https://github.com/swalker2000/duster_broker
Example consumer based on esp32: https://github.com/swalker2000/duster_esp32_example

To compile, you need to create a `Secret.h` file in the main sketch directory with the following content:

```c++
#define SSID          "MY_SSID"
#define WIFI_PASS     "MY_WIFI_PASS"
#define URL           "MQTT_URL"
#define PORT          8883
#define MQTT_USERNAME "MQTT_USERNAME"
#define MQTT_PASS     "MQTT_PASSWORD"
```