/**
 * Classes representing the engine and gear box. These are not with Car mostly to reduce
 * the amount of code in that class, this functionality can be nicely removed.
 */
#pragma once

#include "curve.h"
#include "car.h"

class Car;

class Engine {
    public:
        Engine();

        // Setters
        void setMass(float mass);
        void setRpm(float maxRpm, float idleRpm, float stallRpm, float startRpm);
        void setDifferential(float ratio);
        void setTorqueCurve(Curve & curve, float maxTorque);
        void setRpm(float rpm);

        // Getters
        float & getCurrentRpm();
        float getStallRpm();
        float getMaxRpm();
        float getIdleRpm();

        // These functions adjust the engine RPM based on an elapsed time period. Either
        // with the accelerator pressed or not. If the accelerator was pressed, the RPM
        // will increase, otherwise it will decrease. Time is in seconds.
        void accelerate(float time);
        void decelerate(float time);

        // Calculate the torque produced by the engine at this time
        float calculateTorque();

        void print();

    private:
        // The engine mass
        float _mass;

        // The maximum RPM of the engine (redline)
        float _maxRpm;

        // The idle RPM
        float _idleRpm;

        // The stall RPM
        float _stallRpm;

        // The start RPM 
        float _startRpm;

        // The current RPM of the engine
        float _currentRpm;

        // The differential ratio
        float _differentialRatio;

        // The torque curve
        Curve _torqueCurve;

        // The max torque (the torque curve gives normalised values)
        float _maxTorque;

        // The amount the RPM will change in a second of having the accelerator pressed
        // or not (negative)
        // TODO: This is a very simplified model, in reality there is feedback from the 
        // rest of the system, driven by engine torque
        float _rpmDelta;
};

class Gearbox {
    public:
        Gearbox();
        ~Gearbox();

        Gearbox & operator=(const Gearbox & other);

        // Get the current gear ratio
        float getCurrentRatio();

        // Get the current gear
        int getCurrentGear() { return this->_currentGear; }

        // Do a shift up / down if necessary based on the current engine rpm
        void doShift();

        // Set the number of gears the the gear ratio array
        void setNGears(int number);

        // Set a particular gear ratio
        void setGearRatio(int gear, float ratio);

        // Set the shift up / down rpms
        void setShiftRpms(float up, float down);

        void setCar(Car * car);

        // Print out this gearbox's settings
        void print();
    private:
        int _nGears;
        // In this array 0 is reverse 1 is first, 2 is second etc up to _nGears
        float * _gearRatios;
        float _shiftUpRpm;
        float _shiftDownRpm;

        // The current gear. -1 is neutral
        int _currentGear;

        // This gearbox's car
        Car * _car;

        // Shift up and down
        void _shiftDown();
        void _shiftUp();
};
