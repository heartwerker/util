#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <functional>
#include "config.h"

class MQTT
{
public:
    MQTT(Client &client, const char *server, int port)
        : client(client), server(server), port(port) {}

    void setup();
    void loop();
    void sendComponent(String component_name, String value);
    void sendLightBrightness(String component_name, uint8_t brightness);
    void sendLight(String component_name, int value);

    using LightChangeCallback = std::function<void(const String &name, float percent)>;
    void setLightChangeCallback(LightChangeCallback callback);

    struct Component
    {
        String type;
        String name;
    };

private:
    PubSubClient client;
    const char *server;
    int port;

    bool _subscribed = false;

    void reconnect();
    void publishDiscoveryMessage(Component component);
    void handleCallback(char *topic, byte *payload, unsigned int length);

    LightChangeCallback lightChangeCallback;

    const String discovery_prefix = "homeassistant";
    const String device_name = DEVICE_NAME;

    String device_id;

    Component components[7] = {
        // {"sensor", "rotation"},
        // {"sensor", "position"},
        {"light", "rotation_CW"},
        {"light", "rotation_CCW"},
        {"light", "rotation_animation_period"},
        {"light", "rotation_animation_amplitude"},
        {"light", "position"},
        {"light", "position_animation_period"},
        {"light", "position_animation_amplitude"}
        // {"switch", "relay"}
    };

    String getStateTopicFromComponents(Component component) { return getBaseFromComponent(component) + "/state"; }
    String getCommandTopicFromComponents(Component component) { return getBaseFromComponent(component) + "/set"; }
    String getBaseFromComponent(Component component) { return device_name + "/" + component.type + "/" + component.name; }

    void publishState(const String &topic, const DynamicJsonDocument &stateDoc);
    void processLightCommand(const String &component_name, const DynamicJsonDocument &doc);
    void reactToLightChange(const String &component_name, uint8_t brightness);
};

void MQTT::setup()
{
    client.setBufferSize(1024);
    client.setServer(server, port);
    client.setCallback([this](char *topic, byte *payload, unsigned int length)
                       { this->handleCallback(topic, payload, length); });

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    device_id = device_name + String("_") + mac;

    reconnect();
}

void MQTT::loop()
{
    if (!client.connected())
        reconnect();

    client.loop();

    if (client.connected() && !_subscribed)
    {
        printf("Subscribing to topics\n");

        for (const auto &component : components)
        {
            if (component.type == "light")
            {
                printf("Subscribing to %s\n", getCommandTopicFromComponents(component).c_str());
                client.subscribe(getCommandTopicFromComponents(component).c_str());
            }
        }
        _subscribed = true;
        printf("Subscribed to all topics\n");
    }
}

void MQTT::sendComponent(String component_name, String value)
{
    for (const auto &component : components)
    {
        if (component.name == component_name)
        {
            String topic = getStateTopicFromComponents(component);

            DynamicJsonDocument stateDoc(200);
            stateDoc["value"] = value;

            publishState(topic, stateDoc);
            return;
        }
    }
    Serial.println("Error: Component not found");
}

void MQTT::sendLightBrightness(String component_name, uint8_t brightness)
{
    for (const auto &component : components)
    {
        if (component.name == component_name && component.type == "light")
        {
            String topic = getStateTopicFromComponents(component);

            DynamicJsonDocument stateDoc(200);
            stateDoc["state"] = brightness > 0 ? "ON" : "OFF";
            stateDoc["brightness"] = brightness;

            printf("sendLight %s: %d\n", component_name.c_str(), brightness);

            publishState(topic, stateDoc);
            return;
        }
    }
    Serial.println("Error: Component not found");
}

void MQTT::sendLight(String component_name, int value)
{
    sendLightBrightness(component_name, util::mapConstrainf(value, 0, 100, 0, 255));
}

void MQTT::setLightChangeCallback(LightChangeCallback callback)
{
    lightChangeCallback = callback;
}

void MQTT::publishDiscoveryMessage(Component component)
{
    String discovery_topic = discovery_prefix + "/" + component.type + "/" + component.name + "/config";
    String state_topic = getStateTopicFromComponents(component);

    DynamicJsonDocument doc(1024);

    JsonObject device = doc.createNestedObject("device");
    device["name"] = device_name;
    device["ids"] = device_id;
    device["mf"] = "heartwerk.tech";
    device["mdl"] = device_name;
    device["sw"] = "0.1";
    device["hw"] = "0.1";

    String name = component.name;
    doc["name"] = name;
    doc["unique_id"] = name + "_" + WiFi.macAddress();
    if (component.type == "light")
    {
        doc["~"] = getBaseFromComponent(component);
        doc["cmd_t"] = "~/set";
        doc["stat_t"] = "~/state";
        doc["schema"] = "json";
        doc["brightness"] = true;
    }
    else
    {
        doc["state_topic"] = state_topic;
    }

    doc["unit_of_measurement"] = "";
    doc["value_template"] = "{{ value_json.value }}";

    char buffer[1024];
    serializeJson(doc, buffer);

    if (client.publish(discovery_topic.c_str(), buffer, true))
        Serial.println("Config published for auto-discovery");
    else
        Serial.println("Failed to publish config");
}

void MQTT::handleCallback(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);

    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
    }

    String topicStr = String(topic);

    if (topicStr.indexOf("/light/") != -1)
        for (const auto &component : components)
            if (component.type == "light")
            {
                String componentNameInTopic = topicStr.substring(topicStr.indexOf("/light/") + 7);
                componentNameInTopic = componentNameInTopic.substring(0, componentNameInTopic.indexOf("/set"));

                // printf("Comparing %s with %s\n", componentNameInTopic.c_str(), component.name.c_str());
                if (componentNameInTopic == component.name)
                {
                    processLightCommand(component.name, doc);
                    return;
                }
            }
}

void MQTT::reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(device_id.c_str())) // THIS HAS TO BE UNIQUE per device
        {
            Serial.println("connected");

            for (const auto &component : components)
                publishDiscoveryMessage(component);

            _subscribed = false;
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(2000);
        }
    }
}

void MQTT::publishState(const String &topic, const DynamicJsonDocument &stateDoc)
{
    String stateJson;
    serializeJson(stateDoc, stateJson);

    if (client.publish(topic.c_str(), stateJson.c_str()))
    {
        // Serial.println("State published successfully");
    }
    else
        Serial.println("Failed to publish state");
}

void MQTT::processLightCommand(const String &component_name, const DynamicJsonDocument &doc)
{
    if (doc.containsKey("state"))
    {
        String state = doc["state"];
        Serial.print("State: ");
        Serial.println(state);

        if (state == "OFF")
        {
            reactToLightChange(component_name, 0);
        }
        else if (state != "ON")
            Serial.println("Invalid state value");
    }
    else
        Serial.println("State key not found in JSON");

    if (doc.containsKey("brightness"))
    {
        uint8_t brightness = doc["brightness"];

        Serial.print("Brightness: ");
        Serial.println(brightness);

        reactToLightChange(component_name, brightness);
    }
    else
        Serial.println("Brightness key not found in JSON");
}

void MQTT::reactToLightChange(const String &component_name, uint8_t brightness)
{
    // loopback to mqtt
    sendLightBrightness(component_name, brightness);

    // use elsewhere via callback
    float percent = brightness / 255.0;
    if (lightChangeCallback)
        lightChangeCallback(component_name, percent);
}