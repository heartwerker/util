#pragma once

namespace util {

#define MIN_TO_MS 60000

int sign(float value) { return (value > 0) - (value < 0); }

template <typename T>
static T map(T value, float inMin, float inMax, float outMin, float outMax)
{
    return (value - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
}

float clipf(float value, float low, float high)
{
    if (value < low)
        return low;
    if (value > high)
        return high;
    return value;
}

template <typename T>
T clip(T value, T low, T high) { return constrain(value, low, high); }

#ifndef ARDUINO 
template <typename T>
T clip(T value, T low, T high) {
    if (value < low)
 {
    if (value < low)
        return low;
    if (value > high)
        return high;
    return value;
}
#endif


float mapf(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
    return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

float mapConstrainf(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
    return constrain(mapf(value, fromLow, fromHigh, toLow, toHigh), toLow, toHigh);
}

float mapConstrainf_withCenter(float value, float fromLow, float fromCenter, float fromHigh, float toLow, float toHigh)
{
    if (value == fromCenter)
        return (toLow + (toHigh - toLow) / 2.0f);
    else if (value < fromCenter)
        return mapConstrainf(value, fromLow, fromCenter, toLow, 0);
    else
        return mapConstrainf(value, fromCenter, fromHigh, 0, toHigh);
}

template <typename T>
T wrap(T value, T low, T high) {
    T range = high - low;
    while (value < low)
        value += range;
    while (value >= high)
        value -= range;
    return value;
}

float wrapf(float value, float low, float high) {
    return wrap(value, low, high);
}

float normf(float value, float low, float high) {
    return constrain((value - low) / (high - low), 0, 1);
    // return (value - low) / (high - low);
}


float convert_zero_zone(float value, float zero_zone)
{
    float sign = value > 0 ? 1 : -1;

    value = fabs(value);
    value -= zero_zone;
    value /= (1.0 - zero_zone);

    value = clipf(value, 0, 1);

    return value * sign;
}



} // namespace util