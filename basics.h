#pragma once

namespace util {

#define MIN_TO_MS 60000


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


} // namespace util