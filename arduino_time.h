#pragma once
#include <Arduino.h>
#include <elapsedMillis.h>

using lpsd_ms = elapsedMillis;
using time_ms = uint32_t;

namespace util {

time_ms now_ms()
{
    return millis();
}

time_ms since_ms(time_ms event_ms)
{
    return now_ms() - event_ms;
}

}