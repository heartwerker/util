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
    void sendLight(String component_name, uint8_t brightness);

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

    void reconnect();
    void publishDiscoveryMessage(Component component);
    void handleCallback(char *topic, byte *payload, unsigned int length);

    LightChangeCallback lightChangeCallback;

    const String discovery_prefix = "homeassistant";
    const String device_name = DEVICE_NAME;

    Component components[4] = {
        {"sensor", "rotation"},
        {"sensor", "position"},
        {"light", "light_rotation"},
        {"light", "light_position"},
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
    reconnect();
}

void MQTT::loop()
{
    if (!client.connected())
        reconnect();

    client.loop();
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

void MQTT::sendLight(String component_name, uint8_t brightness)
{
    for (const auto &component : components)
    {
        if (component.name == component_name && component.type == "light")
        {
            String topic = getStateTopicFromComponents(component);

            DynamicJsonDocument stateDoc(200);
            stateDoc["state"] = brightness > 0 ? "ON" : "OFF";
            stateDoc["brightness"] = brightness;

            publishState(topic, stateDoc);
            return;
        }
    }
    Serial.println("Error: Component not found");
}

void MQTT::setLightChangeCallback(LightChangeCallback callback)
{
    lightChangeCallback = callback;
}

void MQTT::publishDiscoveryMessage(Component component)
{
    String discovery_topic = discovery_prefix + "/" + component.type + "/" + component.name + "/config";
    String state_topic = getStateTopicFromComponents(component);

    String UNIQUE_MAC = WiFi.macAddress();
    UNIQUE_MAC.replace(":", "");

    String device_id = device_name + String("_") + UNIQUE_MAC;

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

    if (String(topic).indexOf("/light/") != -1)
        for (const auto &component : components)
        {
            if (component.type == "light" && String(topic).indexOf(component.name) != -1)
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
        if (client.connect("ESP32Client"))
        {
            Serial.println("connected");

            for (const auto &component : components)
            {
                publishDiscoveryMessage(component);
                if (component.type == "light")
                {
                    client.subscribe(getCommandTopicFromComponents(component).c_str());
                }
            }
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void MQTT::publishState(const String &topic, const DynamicJsonDocument &stateDoc)
{
    String stateJson;
    serializeJson(stateDoc, stateJson);

    if (client.publish(topic.c_str(), stateJson.c_str()))
        Serial.println("State published successfully");
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
    sendLight(component_name, brightness);

    // use elsewhere via callback
    float percent = brightness / 255.0;
    if (lightChangeCallback)
        lightChangeCallback(component_name, percent);
}