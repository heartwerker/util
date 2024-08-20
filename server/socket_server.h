#pragma once

#include "managed_server.h"
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

typedef void (*Websocket_Callback)(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

class SocketServer : public ManagedServer
{
public:
    SocketServer() : ManagedServer(), webSocket(81)
    {
    }

    bool setup(const char *name, Websocket_Callback callback)
    {
        Serial.println("Setting up SocketServer");

        if (ManagedServer::setup(name))
        {
            ManagedServer::on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                              { request->send(SPIFFS, "/index.html", "text/html"); });
            ManagedServer::onNotFound([](AsyncWebServerRequest *request)
                                      { request->send(404, "text/plain", "File not found"); });
            ManagedServer::serveStatic("/", SPIFFS, "/");

            webSocket.begin();           // start websocket
            webSocket.onEvent(callback); // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"

            ManagedServer::begin(); // start server after the websocket
        }
        return true;
    }

    void loop()
    {
        ManagedServer::loop();
        webSocket.loop();

// TODO: mechanism to only send when control values changed
#if 0 // Send wake control values every second
        static elapsedMillis timeElapsed;
        if (timeElapsed > 1000)
        {
            timeElapsed = 0;
            if (webSocket.connectedClients() > 0)
                send_wake_control();
        }
#endif
    }

    // TODO why "type" and not "name" ?
    // Sends a JSON object to all connected WebSocket clients
    void sendJson(const Config::Parameter &param)
    {
        if (webSocket.connectedClients() > 0)
        { // Only send if there are connected clients
            StaticJsonDocument<200> doc;
            doc["type"] = param.name;
            doc["value"] = param.value;
            String jsonString;
            serializeJson(doc, jsonString);
            webSocket.broadcastTXT(jsonString);
            Serial.println("Sent JSON: " + jsonString); // Debug output
        }
    }

    // Sends a JSON object to all connected WebSocket clients
    void sendJson(const String &type, const String &value)
    {
        if (webSocket.connectedClients() > 0)
        { // Only send if there are connected clients
            StaticJsonDocument<200> doc;
            doc["type"] = type;
            doc["value"] = value;
            String jsonString;
            serializeJson(doc, jsonString);
            webSocket.broadcastTXT(jsonString);
            Serial.println("Sent JSON: " + jsonString); // Debug output
        }
    }

    #define MAX_ARRAY_LENGTH 100 // Defines the maximum length for JSON arrays
    // Sends a JSON array to all connected WebSocket clients
    void sendJsonArray(const String &type, const float arrayValues[], int length)
    {
        if (webSocket.connectedClients() > 0 && length > 0)
        { // Only send if there are connected clients and array has elements

            String jsonString = "";
            const size_t CAPACITY = JSON_ARRAY_SIZE(MAX_ARRAY_LENGTH) + 100;
            StaticJsonDocument<CAPACITY> doc;
            JsonArray valueArray = doc.createNestedArray("value");

            for (int i = 0; i < length; ++i)
                valueArray.add(arrayValues[i]);

            doc["type"] = type;
            serializeJson(doc, jsonString);
            webSocket.broadcastTXT(jsonString);
        }
    }

private:
    WebSocketsServer webSocket;
};
