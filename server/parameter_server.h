#pragma once

#include "socket_server.h"
#include "parameter_manager.h"

class ParameterServer : public SocketServer
{
public:
    ParameterServer() : SocketServer()
    {
    }

    bool setup(const char *name, Websocket_Callback callback)
    {
        Serial.println("ParameterServer::setup()");
        data.load();

        Serial.printf("Loaded user data.parameters: (%d) \n", data.parameters.size());
        for (auto param : data.parameters)
            Serial.printf("%s: %d\n", param->name.c_str(), param->value);
        
        SocketServer::setup(name, callback);

        return true;
    }

    void loop()
    {
        SocketServer::loop();
    }

    // TODO why "type" and not "name" ?
    // Sends a JSON object to all connected WebSocket clients
    void sendJson(const ParameterManager::Parameter &param)
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

    bool parse(StaticJsonDocument<200> *pDoc, ParameterManager::Parameter *parameter)
    {
        const String type = static_cast<const char *>((*pDoc)["type"]);
        const int value = (*pDoc)["value"];

        if (type == parameter->name)
        {
            parameter->value = int(value);
            data.save();
            sendJson(parameter->name, String(value));
            return true;
        }
        return false;
    }


public:
    ParameterManager data;
};


