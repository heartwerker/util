#pragma once
#include <Arduino.h>

namespace util 
{

void Serial_printFader(float normedValue, int len = 50)
{
        Serial.printf("|");
        for (int i = 0; i < len; i++)
            Serial.printf(i < normedValue * len ? "=" : " ");
        Serial.printf("|");
}

}