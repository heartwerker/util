#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "server/spiffs_helper.h"
#include <vector>

// ==============================
class Config
{
public:
    
    struct Parameter
    {
        String name;
        int value;

        Parameter(Config* parent, const String& name, int value) : 
        name(name), 
        value(value) {
            parent->register_parameter(this);
        }

        Parameter& operator=(int v){
            value = v;
            return *this;
        }
        operator int() const { return value; }
    };

    std::vector<Parameter*> parameters;
    void register_parameter(Parameter *param) { parameters.push_back(param); }

    Parameter light_start   {this, "light_start", 0};
    Parameter light_end     {this, "light_end", 5};
    Parameter music_start   {this, "music_start", 0};
    Parameter music_end     {this, "music_end", 15};

#if ENABLE_WAKE_COFFEE
    Parameter coffee_start{this, "coffee_start", 0};
#endif

    Parameter alarm_snooze_time   {this, "alarm_snooze_time", 5};
    Parameter backup_start  {this, "backup_start", 29};

    Parameter alarm_enabled {this, "alarm_enabled", true};
    Parameter alarm_hour    {this, "alarm_hour", 9};
    Parameter alarm_minute  {this, "alarm_minute", 0};

    // ==============================
    // constants / not changeable
    int blink_display_when_ringing = 0;
    int display_sleep_time_after_s = 10;  // (only sleeps at night otherwise always on)
    int display_sleep_when_after_h = 22; 
    int display_sleep_when_before_h = 9; 
    int setting_inactivity_s = 15;
    int backup_fade_relative_s = 120;
    bool blink_light_when_backup = true;
    // ==============================

    bool save() const
    {
        Serial.println("config.save() !!! ");

        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile)
        {
            Serial.println("Failed to open config file for writing");
            return false;
        }
        DynamicJsonDocument doc(1024);

        for (auto param : parameters)
            doc[param->name] = param->value;

        if (serializeJson(doc, configFile) == 0)
        {
            Serial.println("Failed to write config file");
            configFile.close();
            return false;
        }

        configFile.close();
        return true;
    }

    bool load()
    {
        initFS();
        
        Serial.println("config.load() !!! ");
        File configFile = SPIFFS.open("/config.json", "r");
        if (!configFile)
        {
            Serial.println("Failed to open config file");
            return false;
        }

        size_t size = configFile.size();
        if (size > 1024)
        {
            Serial.println("Config file size is too large");
            configFile.close();
            return false;
        }

        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        configFile.close();

        DynamicJsonDocument doc(1024);
        auto error = deserializeJson(doc, buf.get());
        if (error)
        {
            Serial.println("Failed to parse config file");
            return false;
        }

        for (auto param : parameters)
            param->value = doc[param->name];

        return true;
    }

    bool wasUpdated()
    {
        if (_wasUpdated)
        {
            _wasUpdated = false;
            return true;
        }
        return false;
    }

    void didUpdate() { _wasUpdated = true; }

private:
    bool _wasUpdated = false;
};