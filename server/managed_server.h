#pragma once

#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

#include "minimal_wifimanager.h"
#include "spiffs_helper.h"

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        // request->addInterestingHeader("ANY");
        if (request->method() == HTTP_POST)
            return false;
        else
            return true;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
#if 1
        if (request->method() == HTTP_GET)
        {
            // Serial.println("captive handled GET request");
            request->send(SPIFFS, "/wifimanager.html", "text/html");
        }
        else if (request->method() == HTTP_POST)
        {
            // Serial.println("captive handled POST request");
            for (int i = 0; i < request->params(); i++)
            {
                const AsyncWebParameter *p = request->getParam(i);
                if (p->isPost())
                {
                    String filePath = "/" + p->name() + ".txt";
                    writeFile(filePath.c_str(), p->value().c_str());
                    Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                }
            }
            request->send(200, "text/plain", "Done, will restart! connect to your wifi! ");
            delay(100);
            ESP.restart();
        }
#else
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>");
        response->print("<p>This is out captive portal front page.</p>");
        response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
        response->printf("<p>Try opening <a href='http://%s'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());
        response->print("</body></html>");
        request->send(response);
#endif
    }
};

class ManagedServer : public AsyncWebServer
{
public:
    ManagedServer() : AsyncWebServer(80)
    {
    }

    void begin()
    {
        AsyncWebServer::begin();
        Serial.printf("HTTP server started here: http://%s\n", WiFi.localIP().toString().c_str());
    }

    void setup(const char *name)
    {
        initFS();
        if (!initWiFi()) // start basic captive wifi manager if not connected
        {
            Serial.println("Setting AP (Access Point)");
            WiFi.softAP("AP: " + String(name), NULL);

            dnsServer.start(53, "*", WiFi.softAPIP());

            IPAddress IP = WiFi.softAPIP();
            Serial.printf("AP IP address: %s\n", IP.toString().c_str());

            AsyncWebServer::serveStatic("/", SPIFFS, "/");
            AsyncWebServer::addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP
#if 0
            // Web Server Root URL
            AsyncWebServer::on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                               { request->send(SPIFFS, "/wifimanager.html", "text/html"); });
#endif
#if 1
            AsyncWebServer::on("/", HTTP_POST, [](AsyncWebServerRequest *request)
                               {
                                Serial.println("normal handled POST request");
                for (int i = 0; i < request->params(); i++)
                {
                    const AsyncWebParameter *p = request->getParam(i);
                    Serial.printf("%s set to: %s\n", p->name().c_str(), p->value().c_str());
                    
                    if (p->isPost())
                    {
                        // use param name for fs path
                        String filePath = "/" + p->name() + ".txt";
                        writeFile(filePath.c_str(), p->value().c_str());
                        Serial.printf("%s set to: %s\n", p->name().c_str(), p->value().c_str());

                        // Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                    }
                }
                request->send(200, "text/plain", "Done, will restart! connect to your wifi! ");
                delay(1000);
                ESP.restart(); });

#endif
            AsyncWebServer::begin();
        }
        else
        {
            _soft_AP_active = false;
        }

        if (!MDNS.begin(name))
            Serial.println("Error setting up mDNS responder!");
        else
            Serial.println("mDNS responder started");
    }

    void loop()
    {
        if (_soft_AP_active)
            dnsServer.processNextRequest();
    }

private:
    bool _soft_AP_active = true;
    DNSServer dnsServer;
};