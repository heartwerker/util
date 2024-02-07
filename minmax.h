#pragma once
#include <Arduino.h>

namespace util 
{
    
    void Serial_print_minmax(float value, float reset_factor = 0.0001)
    {
        static float min = 1;
        static float max = 0;

        if (value < min)
            min = value;
        if (value > max)
            max = value;

        // slowly reset
        min += (1 - min) * reset_factor;
        max -= (max - 0) * reset_factor;

        Serial.printf("min: %f \t max: %f \t value: %f ", min, max, value);
        Serial.println();
    }
}