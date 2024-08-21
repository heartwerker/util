#pragma once
#if ESP32

#include <Arduino.h>
#include <elapsedMillis.h>

#include "../util.h"
using namespace util;

#include "../esp32_pwm.h"

#define PWM_FRQUENCY 150
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
        ESP32_PWM::init(PWM_FRQUENCY);

        pinMode(_pins.ENABLE, OUTPUT);
        digitalWrite(_pins.ENABLE, HIGH);

        pinMode(_pins.L_IS, INPUT);
        pinMode(_pins.R_IS, INPUT);
    }

    void loop()
    {
        if (_since_loop >= 5)
        {
            _since_loop = 0;
            applySpeed();
#if 0
            if (_since_reverse > 5000)
            {
                _since_reverse = 0;
                setSpeed(-_target);
            }
#endif
        }
        if (_since_us_current >= 50)
        {
            _since_us_current = 0;
#define filter_val 0.0001
            // simpleFilterf(_current_L, float(analogRead(_pins.L_IS)) / 4095.0, filter_val);
            // simpleFilterf(_current_R, float(analogRead(_pins.R_IS)) / 4095.0, filter_val);
            simpleFilterf(_current_L, float(analogRead(_pins.L_IS)) / 2048.0, filter_val);
            simpleFilterf(_current_R, float(analogRead(_pins.R_IS)) / 2048.0, filter_val);

            float new_current = _current_L +_current_R;
            simpleFilterf(_current_dif, (new_current - _current) * 1000, 0.001);
            _current = new_current;

        }
#if 1
        if (_since_current_print >= 50)
        {
            _since_current_print = 0;
            // printf("Current \t%2.4f \tdif: %2.4f \t\t L/R:\t%2.4f\t%2.4f\n", _current, _current_dif, _current_L, _current_R);
            printf("%2.4f, %2.4f\n", _current, _current_dif); //, _current_L, _current_R);
        }
#endif
    }

    void setSpeed(float percentage)
    {
        _target = constrain(percentage, -1.0f, 1.0f);
    }

    void applySpeed()
    {
        simpleFilterf(_actual, _target, 0.02);
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

        if (_since_update > 1000)
        {
            if (_target == 0)
                _since_update = 800;
            else
                _since_update = 0;
#if 0
            Serial.printf("_target: \t%2.2f _actual: \t%2.2f -> pwm1: \t%2.2f  pwm2: \t%2.2f   \n", _target, _actual, pwm1, pwm2);
#endif
        }
    }

public:
    int dir = 1;
    int index = 0;
    elapsedMillis _since_update = 0;

    bool invert_dir = true;

    float _current_L = 0;
    float _current_R = 0;

    float _current = 0;
    float _current_dif = 0;

private:
    elapsedMicros _since_us_current = 0;
    elapsedMillis _since_current_print = 0;
    elapsedMillis _since_loop = 0;
    elapsedMillis _since_count = 0;
    elapsedMillis _since_reverse = 0;

    float _target = 0; // _target_speed
    float _actual = 0; // _actual_speed

    struct PINS
    {
        uint8_t ENABLE; // = L_EN + R_EN as 1 pin
        uint8_t LPWM;   // direction + speed
        uint8_t RPWM;   // direction + speed
        uint8_t L_IS;   // current sense
        uint8_t R_IS;   // current sense
    };

    PINS _pins;
};

#endif