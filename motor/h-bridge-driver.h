/**
 * @file H_Bridge_Driver.h
 * 
 * This class provides a basic PWM driver for an H-bridge, enabling speed and direction control of a motor.
 * Based on Ardunio.h
 * 
 * Usage:
 * 1. Include the H_Bridge_Driver.h header file.
 * 2. Create an instance of the H_Bridge_Driver class, providing the control pin numbers as parameters to the constructor.
 * 3. Call the begin() function to initialize the control pins.
 * 4. Set the desired speed using the setSpeed() function, providing a percentage value between -1.0 and 1.0.
 * 5. Call the loop() function periodically to update the speed and apply it to the motor.
 * 
 * Example:
 * @code
 * #include "H_Bridge_Driver.h"
 * 
 * H_Bridge_Driver motor(9, 10); // Create an instance of H_Bridge_Driver with control pins 9 and 10
 * 
 * void setup() {
 *   motor.begin(); // Initialize the control pins
 * }
 * 
 * void loop() {
 *   motor.setSpeed(0.5); // Set the speed to 50%
 *   motor.loop(); // apply update speed (run/call at least with 200Hz)
 *   do other stuff
 * }
 * @endcode
 */
 
 
// TODO: why not make this a more generic H-Bridge driver class or at least inherit from it ?!


#pragma once
#include <Arduino.h>
#include "motor-driver-base.h"
#include <elapsedMillis.h>

#include "../util.h"

#define PWM_FREQUENCY 20000
#define PWM_RANGE 256

class H_Bridge_Driver : public MotorDriverBase
{
public:
    H_Bridge_Driver(uint8_t pin1, uint8_t pin2) : MotorDriverBase()
    {
        control_pin1 = pin1;
        control_pin2 = pin2;
    }

    void setup()
    {
        pinMode(control_pin1, OUTPUT);
        pinMode(control_pin2, OUTPUT);

        analogWrite(control_pin1, 0);
        analogWrite(control_pin2, 0);

#if !ESP32
        analogWriteFreq(PWM_FREQUENCY);
        analogWriteRange(PWM_RANGE);
#else
        analogWriteFrequency(PWM_FREQUENCY);
#endif
    }

    void setFilterValue(float value)
    {
        _filterValue = value;
    }

    void begin() // legacy
    {
        setup();
    }

    void loop()
    {
        if (since_loop > 2)
        {
            since_loop = 0;
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
        // printf("H_Bridge_Driver::setSpeed: %2.2f\n", percentage);
        target = constrain(percentage, -1.0f, 1.0f);
    }

    void applySpeed()
    {
        simpleFilterf(current, target, _filterValue);
        float output = (invert_dir ? -1 : 1) * current;

        int pwm1, pwm2 = 0;
        if (output > 0)
        {
            pwm1 = util::mapConstrainf(fabs(output), 0, 1, 0, float(PWM_RANGE - 1));
            pwm2 = 0;
        }
        else
        {
            pwm1 = 0;
            // pwm2 = util::clipf(fabs(output), 0, 1) * float(PWM_RANGE - 1);
            pwm2 = util::mapConstrainf(fabs(output), 0, 1, 0, float(PWM_RANGE - 1));
        }

        analogWrite(control_pin1, pwm1);
        analogWrite(control_pin2, pwm2);

        if (since_update > 100)
        {
            since_update = 0;
            // Serial.printf("H_Bridge_Driver::target: %2.2f - speed: %2.2f -> pwm1: %d  pwm2: %4d  | ", target, current, pwm1, pwm2); Serial.println();
        }
    }

    float getActual() { return current; }
    float getTarget() { return target; }

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

    float target = 0;
    float current = 0;

    float _filterValue = 0.02f;

    uint8_t control_pin1;
    uint8_t control_pin2;
};
