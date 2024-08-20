#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "server/spiffs_helper.h"
#include <vector>

#define PARAMETER_FILE_NAME "/parameter.json"

// ==============================
class ParameterManager
{
public:
    
    struct Parameter
    {
        String name;
        int value;

        Parameter(ParameterManager* parent, const String& name, int value) : 
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
        Serial.println("ParameterManager::save() !!! ");

        File parameterFile = SPIFFS.open(PARAMETER_FILE_NAME, "w");
        if (!parameterFile)
        {
            Serial.println("Failed to open parameter file for writing");
            return false;
        }
        DynamicJsonDocument doc(1024);

        for (auto param : parameters)
            doc[param->name] = param->value;

        if (serializeJson(doc, parameterFile) == 0)
        {
            Serial.println("Failed to write file");
            parameterFile.close();
            return false;
        }

        parameterFile.close();
        return true;
    }

    bool load()
    {
        initFS();
        
        Serial.println("ParameterManager::load() !!! ");
        File parameterFile = SPIFFS.open(PARAMETER_FILE_NAME, "r");
        if (!parameterFile)
        {
            Serial.println("Failed to open file");
            return false;
        }

        size_t size = parameterFile.size();
        if (size > 1024)
        {
            Serial.println("file size is too large");
            parameterFile.close();
            return false;
        }

        std::unique_ptr<char[]> buf(new char[size]);
        parameterFile.readBytes(buf.get(), size);
        parameterFile.close();

        DynamicJsonDocument doc(1024);
        auto error = deserializeJson(doc, buf.get());
        if (error)
        {
            Serial.println("Failed to parse file");
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
