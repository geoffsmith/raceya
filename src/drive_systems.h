/**
 * Classes representing the engine and gear box. These are not with Car mostly to reduce
 * the amount of code in that class, this functionality can be nicely removed.
 */
#pragma once

#include "curve.h"

class Engine {
    public:
        Engine();

        // Setters
        void setMass(float mass);
        void setRpm(float maxRpm, float idleRpm, float stallRpm, float startRpm);
        void setDifferential(float ratio);
        void setTorqueCurve(Curve & curve, float maxTorque);

        // Getters
        float & getCurrentRpm();

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
};
