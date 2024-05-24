#pragma once

/*
 *  MPU6050 / GY-521
 * in platformio.ini add:
 * lib_deps
 * rfetick/MPU6050_light@^1.1.0
 */

#include <Arduino.h>
#include "Wire.h"
#include <MPU6050_light.h>
#include <elapsedMillis.h>

// Default Wire lines for i2c
// D1, D2 used for SDA,SCL 

class GyroMPU6050
{
public:
    void setup()
    {
        Wire.begin();

        byte status = _mpu.begin();
        printf("MPU6050 status: %d\n", status);
        while (status != 0)
        {
            printf("Problem: ");
            printf("MPU6050 status: %d\n", status);
            delay(1000);
            status = _mpu.begin();
            if (status != 0)
                printf("Retrying ... \n");
        } // blocks everything if could not connect to MPU6050

        if (_calibrate)
        {
            printf("Measuring offsets, do not move MPU6050\n");
            delay(1000);
            _mpu.calcOffsets(); // gyro and accelero
            printf("Done!\n\n");
        }
    }

    void loop()
    {
        _mpu.update();

        x = _mpu.getAngleX() / 180.0;
        y = _mpu.getAngleY() / 180.0;
        z = _mpu.getAngleZ() / 180.0;

        if (_debugActive)
            if (_sincePrint > 250)
            {
                _sincePrint = 0;
                // printf("X : %f\tY : %f\tZ : %f\n", _mpu.getAngleX(), _mpu.getAngleY(), _mpu.getAngleZ());
            }
    }

public:
    float x = 0;
    float y = 0;
    float z = 0;

private:
    MPU6050 _mpu{Wire};
    
    elapsedMillis _sincePrint = 0;
    
    bool _debugActive = true;
    bool _calibrate = true;
};
