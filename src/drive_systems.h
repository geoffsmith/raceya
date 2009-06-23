/**
 * Classes representing the engine and gear box. These are not with Car mostly to reduce
 * the amount of code in that class, this functionality can be nicely removed.
 */
#pragma once

#include "curve.h"

class Car;

class Engine {
    public:
        Engine(Car & car);

        // Setters
        void setMass(const float mass);
        void setRpm(
                const float maxRpm, 
                const float idleRpm, 
                const float stallRpm, 
                const float startRpm);
        void setDifferential(const float ratio);
        void setTorqueCurve(const Curve & curve, const float maxTorque);
        void setRpm(const float rpm);

        // Getters
        float & getCurrentRpm();
        float getStallRpm();
        float getMaxRpm();
        float getIdleRpm();
        float getDifferential();

        // These functions adjust the engine RPM based on an elapsed time period. Either
        // with the accelerator pressed or not. If the accelerator was pressed, the RPM
        // will increase, otherwise it will decrease. Time is in seconds.
        void accelerate(float time);
        void decelerate(float time);

        // Calculate the torque produced by the engine at this time
        float calculateTorque();

        // Calculate the torque for a particular RPM
        float calculateTorque(float rpm);

        // Functions to set the accelerator to being pressed or not
        void pressAccelerator();
        void releaseAccelerator();

        void print() const;

    private:
        Car & car;

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

        // The value between 0 and 1 of how much the accelerator is pressed. This is used
        // to linearly scale the torque produced by the engine.
        float accelerator;

        // A value between 0..1 that serves as traction control. It works by damping the 
        // accelerator value. If the slip is too high, this is reduced, if it is too low
        // it is increased.
        float tractionControl;
        float tractionControlDelta;
};

class Gearbox {
    public:
        Gearbox(Car & car);
        ~Gearbox();

        Gearbox(const Gearbox & other);
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

        //void setCar(Car * car);

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
        Car & car;

        // Shift up and down
        void _shiftDown();
        void _shiftUp();
};
