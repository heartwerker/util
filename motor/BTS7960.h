#pragma once

#include <Arduino.h>
#include <elapsedMillis.h>

#include "../util.h"
using namespace util;

#include "../esp32_pwm.h"

// #define PWM_FRQUENCY 20000
// #define PWM_RANGE 1024


struct BTS7960_Driver
{

public:
    BTS7960_Driver(uint8_t ENABLE, uint8_t LPWM, uint8_t RPWM, uint8_t L_IS, uint8_t R_IS)
    {
        _pins.ENABLE = ENABLE;
        _pins.LPWM = LPWM;
        _pins.RPWM = RPWM;
        _pins.L_IS = L_IS;
        _pins.R_IS = R_IS;
    }

    void setup()
    {
        pinMode(_pins.L_IS, INPUT);
        pinMode(_pins.R_IS, INPUT);

        ESP32_PWM::add(_pins.LPWM);
        ESP32_PWM::add(_pins.RPWM);
        ESP32_PWM::init();

        pinMode(_pins.ENABLE, OUTPUT);
        digitalWrite(_pins.ENABLE, HIGH);
    }

    void loop()
    {
        if (since_loop > 5)
        {
            since_loop = 0;
            applySpeed();
#if 0
            if (since_reverse > 5000)
            {
                since_reverse = 0;
                setSpeed(-_target);
            }
#endif
        }
    }

    void setSpeed(float percentage)
    {
        _target = constrain(percentage, -1.0f, 1.0f);
    }

    void applySpeed()
    {
        _actual += (_target - _actual) * 0.0101f;
        // _actual = _target;
        float output = (invert_dir ? -1 : 1) * _actual;

        float pwm1, pwm2 = 0;
        if (output > 0)
        {
            pwm1 = clipf(fabs(output), 0, 1);
            pwm2 = 0;
        }
        else
        {
            pwm1 = 0;
            pwm2 = clipf(fabs(output), 0, 1);
        }
        
        ESP32_PWM::set(_pins.LPWM, pwm1);
        ESP32_PWM::set(_pins.RPWM, pwm2);

        if (since_update > 100)
        {
            since_update = 0;
            Serial.printf("_target: \t%2.2f _actual: \t%2.2f -> pwm1: \t%2.2f  pwm2: \t%2.2f  | ", _target, _actual, pwm1, pwm2);
            Serial.println();
        }
    }

public:
    int dir = 1;
    int index = 0;
    elapsedMillis since_update = 0;

    bool invert_dir = true;

private:
    elapsedMillis since_loop = 0;
    elapsedMillis since_count = 0;
    elapsedMillis since_reverse = 0;

    float _target = 0; // _target_speed
    float _actual = 0; // _actual_speed

    struct PINS {
        uint8_t ENABLE; // = L_EN + R_EN as 1 pin
        uint8_t LPWM; // direction + speed
        uint8_t RPWM; // direction + speed
        uint8_t L_IS; // current sense
        uint8_t R_IS; // current sense
    };

    PINS _pins;
};
