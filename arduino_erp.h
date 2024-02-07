#pragma once

#include "util.h"

#include "generic_erp.h"
#include <Arduino.h>

namespace util
{

struct ArduinoErp : public GenericErp
{
    void begin(int pin_a, int pin_b, float min = 0.0, float max = 1.0)
    {
        pin_adc[0] = pin_a;
        pin_adc[1] = pin_b;

        for (int i = 0; i < 2; i++)
        {
            pinMode(pin_adc[i], INPUT);
            set_MinMax(i, min, max);
        }
    }

    void process()
    {
        float adc[2];
        for (int i = 0; i < 2; i++)
            adc[i] = analogRead(pin_adc[i]) / 4095.0;

        GenericErp::process(adc[0], adc[1]);
    }

    void print_debug()
    {
        for (int i = 0; i < 2; i++)
            Serial.printf("%d:: ch %.3f,  norm %.3f,  min: %.3f,  max: %.3f \t", i, ch[i], ch_norm[i], ch_min[i], ch_max[i]);
    }

private:
    int pin_adc[2] = {0, 0};
};

} // namespace util