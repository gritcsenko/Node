#include <MqttConnector.h>

MqttConnector* init_mqtt(String host, int port, String channelPrefix, String userName, String password, String& clientId, std::function<void(MqttConnector*)> publish_hooks, std::function<void(MqttConnector*)> receive_hooks)
{
  MqttConnector* mqtt = new MqttConnector(host.c_str(), port);

  mqtt->on_connecting([&](int counter, bool * flag) {
    MQTT_DEBUG_PRINTLN("[%lu] MQTT CONNECTING.. ", counter);
    if (counter >= 600) {
      ESP.reset();
    }
    delay(1000);
  });

  mqtt->on_prepare_configuration([&](MqttConnector::Config * config) -> void {
    clientId = ESP.getChipId();
    config->clientId  = clientId;
    config->channelPrefix = channelPrefix;
    config->enableLastWill = true;
    config->retainPublishMessage = true;
    /*
        config->mode
        ===================
        | MODE_BOTH       |
        | MODE_PUB_ONLY   |
        | MODE_SUB_ONLY   |
        ===================
    */
    config->mode = MODE_BOTH;
    config->firstCapChannel = false;

    config->username = String(userName);
    config->password = String(password);

    // FORMAT
    // d:quickstart:<type-id>:<device-id>
    //config->clientId  = String("d:quickstart:esp8266meetup:") + macAddr;
    //config->topicPub  = String("iot-2/evt/status/fmt/json");
  });

  mqtt->on_after_prepare_configuration([&](MqttConnector::Config config) -> void {
    MQTT_DEBUG_PRINTLN("[USER] HOST = %s", config.mqttHost.c_str());
    MQTT_DEBUG_PRINTLN("[USER] PORT = %d", config.mqttPort);
    MQTT_DEBUG_PRINTLN("[USER] PUB  = %s", config.topicPub.c_str());
    MQTT_DEBUG_PRINTLN("[USER] SUB  = %s", config.topicSub.c_str());
  });

  publish_hooks(mqtt);
  receive_hooks(mqtt);

  mqtt->connect();
}
