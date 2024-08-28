#pragma once

#include "socket_server.h"
#include "parameter_data.h"

class ParameterServer : public SocketServer
{
public:
    ParameterServer() : SocketServer()
    {
    }
    bool setup(const char *name, Websocket_Callback callback)
    {
        Serial.println("ParameterServer::setup()");
        if (pData == nullptr)
            Serial.println("ParameterServer::PROBLEMMMMMMMMM()");
        
        
        pData->load();

        Serial.printf("Loaded user pData->parameters: (%d) \n", pData->parameters.size());
        for (auto param : pData->parameters)
            Serial.printf("%s: %d\n", param->name.c_str(), param->value);
        
        SocketServer::setup(name, callback);

        return true;
    }

    void loop()
    {
        SocketServer::loop();
        for (auto param : pData->getParameter_changed_from_code())
            sendJson(param);
    }

    // void sendJson(const ParameterData::Parameter &param) { sendJson(&param); }
    void sendJson(const ParameterData::Parameter &param)
    {
        if (webSocket.connectedClients() > 0)
        { // Only send if there are connected clients
            StaticJsonDocument<200> doc;
            doc["type"] = param.name;
            doc["value"] = param.value;
            String jsonString;
            serializeJson(doc, jsonString);
            webSocket.broadcastTXT(jsonString);
#if DEBUG_SERVER
            Serial.println("Sent JSON: " + jsonString); // Debug output
#endif
        }
    }
    // TODO why "type" and not "name" ?
    // Sends a JSON object to all connected WebSocket clients
    void sendJson(const ParameterData::Parameter* pParam)
    {
        if (webSocket.connectedClients() > 0)
        { // Only send if there are connected clients
            StaticJsonDocument<200> doc;
            doc["type"] = pParam->name;
            doc["value"] = pParam->value;
            String jsonString;
            serializeJson(doc, jsonString);
            webSocket.broadcastTXT(jsonString);
#if DEBUG_SERVER
            Serial.println("Sent JSON: " + jsonString); // Debug output
#endif
        }
    }

    // Sends a JSON object to all connected WebSocket clients
    void sendJson(const String &type, const String &value)
    {
        Serial.printf("OBSOLETE. dont use anymore!!");
        
        if (webSocket.connectedClients() > 0)
        { // Only send if there are connected clients
            StaticJsonDocument<200> doc;
            doc["type"] = type;
            doc["value"] = value;
            String jsonString;
            serializeJson(doc, jsonString);
            webSocket.broadcastTXT(jsonString);
#if DEBUG_SERVER
            Serial.println("Sent JSON: " + jsonString); // Debug output
#endif
        }
    }

    #define MAX_ARRAY_LENGTH 100 // Defines the maximum length for JSON arrays
    // Sends a JSON array to all connected WebSocket clients
    void sendJsonArray(const String &type, float * arrayValues, int length)
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

    void sendAllParameters()
    {
        for (auto param : pData->parameters)
            sendJson(*param);
    }

    bool parse(StaticJsonDocument<200> *pDoc, ParameterData::Parameter *parameter)
    {
        const String type = static_cast<const char *>((*pDoc)["type"]);
        const int value = (*pDoc)["value"];

        if (type == parameter->name)
        {
            parameter->value = int(value);
            pData->save();
            sendJson(parameter->name, String(value));
            return true;
        }
        return false;
    }


public:
    ParameterData *pData = nullptr;
};


