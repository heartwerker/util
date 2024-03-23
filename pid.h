#pragma once
#include "basics.h"

using namespace util;

class PID
{
public:
    float process(float in)
    {
        _input = in;
        float error = _target - _input;
        float delta = _input - _input_z1;
        _output = process_internal(error, delta);

        _input_z1 = _input;
        
        return _output;
    }

    float process(float in, float t)
    {
        setTarget(t);
        return process(in);
    }

    // TODO maybe subclass this.
    float process_unwrap(float in)
    {
        _input = in;
        float error = wrap(_target - _input, -0.5f, 0.5f);
        float delta = wrap(_input - _input_z1, -0.5f, 0.5f);
        _output = process_internal(error, delta);

        _input_z1 = _input;
        
        return _output;
    }

    float process_internal(float error, float delta_in)
    {
        _output_ki += _ki * error / float(_processRate);
        _output_ki = clipf(_output_ki, -20, 20);

        _output = 0;
        _output += _kp * error;
        _output += _output_ki;
        _output += _kd * delta_in * float(_processRate) / 1000.0;

        _input_z1 = _input;

        _kp = clipf(_kp, 0, 100000);
        _ki = clipf(_ki, 0, 100000);
        
        return _output;
    }

    void setParams(float Kp, float Ki, float Kd)
    {
        _kp = Kp;
        _ki = Ki;
        _kd = Kd;
    }
    void setSampleRate(int rate) { _processRate = rate; }

    void reset()
    {
        _output_ki = 0;
        _input_z1 = _input;
    }

    void setTarget(float t) { _target = t; }

public:
    float _target = 0;

    float _kp; // * (P)roportional Tuning Parameter
    float _ki; // * (I)ntegral Tuning Parameter
    float _kd; // * (D)erivative Tuning Parameter

    float _input = 0;
    float _output = 0;

    float _output_ki, _input_z1;

    int _processRate = 1000; // = sample rate
};
