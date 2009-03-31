/**
 * Classes representing the engine and gear box. These are not with Car mostly to reduce
 * the amount of code in that class, this functionality can be nicely removed.
 */
#pragma once

class Engine {
    public:
        Engine();

        // Setters
        void setMass(float mass);
        void setRpm(float maxRpm, float idleRpm, float stallRpm, float startRpm);
        void setDifferential(float ratio);

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

        // The differential ratio
        float _differentialRatio;
};
