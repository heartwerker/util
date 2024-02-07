#pragma once

#include "util.h"
using namespace util;

// TODO idealy remove Arduino dependency here:
#include <Arduino.h> // for isnan 

#define MAX_NUM_SENSORS 2

struct GenericErp
{
    float position()
    {
        if (isnan(_position))
            return 0;
        return _position;
    }

    /**
     * @brief Calculates the change since last call
     * @param threshold The minimal change to be detected.
     * @return The delta value.
     */
    float delta(float threshold)
    {
        if (isnan(_position))
            return 0;

        if (fabs(_delta) > threshold)
        {
            _last_activity_time = now_ms();

            float d = _delta;
            _delta = 0;

            if (!_delta_available)
            {
                _delta_available = true; // effect on next call
                return 0;
            }
            return d;
        }

        return 0;
    }

    void process(float ch_a, float ch_b)
    {
        ch[0] = ch_a;
        ch[1] = ch_b;

        for (int i = 0; i < MAX_NUM_SENSORS; i++)
        {
            // TODO filter adc input maybe ?!
            
            // update min max.
            if (ch[i] < ch_min[i])
                ch_min[i] = ch[i];

            if (ch[i] > ch_max[i])
                ch_max[i] = ch[i];

            ch_norm[i] = util::normf(ch[i], ch_min[i], ch_max[i]);
        }
        _position = atan2(ch_norm[0] - 0.5, ch_norm[1] - 0.5);
        _position = util::normf(_position, -PI, PI);

        float delta = _position - _position_z1;
        _position_z1 = _position;

        if (isnan(delta))
            delta = 0;

        _delta += util::wrapf(delta, -0.5, 0.5);
    }

    void set_MinMax(int i, float min, float max)
    {
        ch_min[i] = min;
        ch_max[i] = max;
    }

protected:
    float _position = 0;
    float _position_z1 = 0;

    float _delta = 0;
    float _delta_available = false;
    
    time_ms _last_activity_time = 0;

    float ch[MAX_NUM_SENSORS];
    float ch_norm[MAX_NUM_SENSORS];
    float ch_min[MAX_NUM_SENSORS];
    float ch_max[MAX_NUM_SENSORS];
    float ch_amp[MAX_NUM_SENSORS];

};