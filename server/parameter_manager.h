#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "server/spiffs_helper.h"
#include <vector>

#define PARAMETER_FILE_NAME "/parameter.json"


// ---------------------------------------------------------------------------------------
// Server utilizes WiFi, WebSockets, and JSON for configuration and control.
//
// Requires SPIFFS for file system operations. For setup, refer to:
// https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/
// https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
//
// Icon from: https://icons8.com/icons/set/favicon
// Based on code by mo thunderz, updated last on 11.09.2022.
// https://github.com/mo-thunderz/Esp32WifiPart4
// ---------------------------------------------------------------------------------------

// ==============================
class ParameterManager
{
public:
    
    struct Parameter
    {
        String name;
        int value;

        Parameter(ParameterManager* parent, const String& name, int default_value) : 
        name(name), 
        value(default_value) {
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
            save();
            return true;
        }
        return false;
    }


    bool parseAll(uint8_t *payload)
    {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return false;
        }

        const String type = static_cast<const char *>(doc["type"]);
        const int value = doc["value"];

        Serial.printf("Command: %s\nValue: %d\n", type.c_str(), value);

        for (auto param : parameters)
        {
            if (type == param->name)
            {
                if (param->value != value)
                {
                    param->value = value;
                    _wasUpdated = true;
                    mark_parameter_changed(param);
                    Serial.printf("Updated %s to %d\n", type.c_str(), value);
                }
            }
        }
        return _wasUpdated;
    }

private:
    bool _wasUpdated = false;

public:
    std::vector<Parameter *> _changedParameters;
    void mark_parameter_changed(Parameter *param)
    {
        if (std::find(_changedParameters.begin(), _changedParameters.end(), param) == _changedParameters.end()) {
            _changedParameters.push_back(param);
        }
    }

    std::vector<Parameter *> getChangedParameters()
    {
        std::vector<Parameter *> changedParameters = _changedParameters;
        _changedParameters.clear();
        return changedParameters;
    }
};


using ParameterList = std::vector<ParameterManager::Parameter *>;
