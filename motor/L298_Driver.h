#pragma once

#include <Arduino.h>
#include <elapsedMillis.h>

#define PWM_FRQUENCY 8000
#define PWM_RANGE 1024
class L298_Driver
{
public:
    L298_Driver(uint8_t pin1, uint8_t pin2)
    {
        control_pin1 = pin1;
        control_pin2 = pin2;
    }

    void begin()
    {
        pinMode(control_pin1, OUTPUT);
        pinMode(control_pin2, OUTPUT);
#if !ESP32
        analogWriteFreq(PWM_FRQUENCY);
        analogWriteRange(PWM_RANGE);
#endif
    }

    void computeSpeed()
    {
        if (since_count > 1000)
        {
            // compute speed as counts / since_count and print result
            speed += ((float(counter_speed_pulse) / float(since_count)) - speed) * 0.2f;
            since_count = 0;
            counter_speed_pulse = 0;

            // Serial.printf("speed: %f\n", speed);
        }
    }

    void loop()
    {
        if (since_loop > 5)
        {
            since_loop = 0;
            computeSpeed();
            applySpeed();

#if 0
            if (since_reverse > 5000)
            {
                since_reverse = 0;
                setSpeed(-target);
            }
#endif
        }
    }

    void setSpeed(float percentage)
    {
        percentage = constrain(percentage, -1.0f, 1.0f);
        target = percentage;
    }

    void applySpeed()
    {
        current += (target - current) * 0.0201f;
        float output = (invert_dir ? -1 : 1) * current;

        int pwm1, pwm2 = 0;
        if (output > 0)
        {
            pwm1 = clipf(fabs(output), 0, 1) * float(PWM_RANGE - 1);
            pwm2 = 0;
        }
        else
        {
            pwm1 = 0;
            pwm2 = clipf(fabs(output), 0, 1) * float(PWM_RANGE - 1);
        }

        analogWrite(control_pin1, pwm1);
        analogWrite(control_pin2, pwm2);

        if (since_update > 100)
        {
            since_update = 0;
            Serial.printf("target: %2.2f - speed: %2.2f -> pwm1: %d  pwm2: %4d  | ", target, current, pwm1, pwm2);
            Serial.println();
        }
    }

public:
    float percent = 0;
    int dir = 1;
    int index = 0;
    elapsedMillis since_update = 0;

    uint8_t feedback_speed_pin;
    int counter_speed_pulse = 0;

    bool invert_dir = false;

private:
    elapsedMillis since_loop = 0;
    elapsedMillis since_count = 0;
    elapsedMillis since_reverse = 0;
    float speed = 0;
    float target = 0;
    float current = 0;

    uint8_t control_pin1;
    uint8_t control_pin2;
};
