#include "drive_systems.h"

#include <iostream>

using namespace std;

Engine::Engine() {
}

void Engine::setMass(float mass) {
    this->_mass = mass;
}

void Engine::setRpm(float maxRpm, float idleRpm, float stallRpm, float startRpm) {
    this->_maxRpm = maxRpm;
    this->_idleRpm = idleRpm;
    this->_stallRpm = stallRpm;
    this->_startRpm = startRpm;

    // Set the current RPM as the start, might need to elaborate this later
    this->_currentRpm = startRpm;
}

void Engine::setRpm(float rpm) {
    this->_currentRpm = rpm;
}

void Engine::setDifferential(float ratio) {
    this->_differentialRatio = ratio;
}

void Engine::setTorqueCurve(Curve & curve, float maxTorque) {
    this->_torqueCurve = curve;
    this->_maxTorque = maxTorque;
}

void Engine::print() {
    cout << "RPM - max: " << this->_maxRpm << ", idle: " << this->_idleRpm;
    cout << ", stall: " << this->_stallRpm << ", start: " << this->_startRpm << endl;
}

float & Engine::getCurrentRpm() {
    // This is a hack really. The engine needs redesigning to allow for idling
    if (this->_currentRpm < this->_idleRpm) {
        this->_currentRpm = this->_idleRpm;
    }

    return this->_currentRpm;
}

float Engine::getStallRpm() {
    return this->_stallRpm;
}

float Engine::getMaxRpm() {
    return this->_maxRpm;
}

float Engine::calculateTorque() {
    // Our torque equation here is torque * gear ratio * differential ratio * efficiency
    float torque = this->_torqueCurve[this->_currentRpm] * this->_maxTorque;
    return torque * this->_differentialRatio * 0.7;
}

/******************************************************************************
 * The gearbox
 *****************************************************************************/
Gearbox::Gearbox() {
    this->_gearRatios = NULL;

    // Set the current gear to neutral
    this->_currentGear = -1;
}

Gearbox::~Gearbox() {
    if (this->_gearRatios != NULL) delete [] this->_gearRatios;
}

Gearbox & Gearbox::operator=(const Gearbox & other) {
    this->_nGears = other._nGears;
    this->_gearRatios = new float[this->_nGears];
    for (int i = 0; i < this->_nGears; ++i) {
        this->_gearRatios[i] = other._gearRatios[i];
    }

    this->_shiftDownRpm = other._shiftDownRpm;
    this->_shiftUpRpm = other._shiftUpRpm;
}

void Gearbox::setNGears(int number) {
    this->_nGears = number;
    this->_gearRatios = new float[this->_nGears];
}

void Gearbox::setGearRatio(int gear, float ratio) {
    this->_gearRatios[gear] = ratio;
}

void Gearbox::setShiftRpms(float up, float down) {
    this->_shiftUpRpm = up;
    this->_shiftDownRpm = down;
}

void Gearbox::setCar(Car * car) {
    this->_car = car;
}

void Gearbox::print() {
    cout << "Gearbox: ";
    for (int i = 0; i < this->_nGears; ++i) {
        cout << i << " -> " << this->_gearRatios[i] << ", ";
    }
    cout << endl;
    cout << "Shift up: " << this->_shiftUpRpm;
    cout << ", shift down: " << this->_shiftDownRpm << endl;
}

float Gearbox::getCurrentRatio() {
    if (this->_currentGear == -1) {
        return 0;
    } else {
        float ratio = this->_gearRatios[this->_currentGear];
        return ratio;
    }
}

void Gearbox::doShift() {
    float currentRpm = this->_car->getEngine()->getCurrentRpm();
    if (this->_currentGear == -1) {
        // If we have the min rpm, shift into first
        if (currentRpm >= this->_shiftDownRpm) {
            this->_currentGear = 1;
        }
    } else {
        // Check if we need to shift down
        if (currentRpm <= this->_shiftDownRpm) {
            --this->_currentGear;
            this->_car->getEngine()->setRpm(this->_shiftUpRpm - 100);

            // IF the new current gear is 0, we set to neutral
            if (this->_currentGear == 0) {

                this->_car->getEngine()->setRpm(this->_shiftDownRpm - 100);
                this->_currentGear = -1;
            }
        } else if (currentRpm >= this->_shiftUpRpm && this->_currentGear < this->_nGears - 1) {
            ++this->_currentGear;
            this->_car->getEngine()->setRpm(this->_shiftDownRpm + 100);
        }
    }
}
