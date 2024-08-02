/**
 * @file L298_Driver.h
 * 
 * This class provides a basic PWM driver for an H-bridge, enabling speed and direction control of a motor.
 */
 
#pragma once
#include "h-bridge-driver.h"

class L298_Driver : public H_Bridge_Driver
{
public:
    L298_Driver(uint8_t pin1, uint8_t pin2) : H_Bridge_Driver(pin1, pin2)
    {
    }

};