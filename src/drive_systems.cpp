#include "drive_systems.h"
#include "car.h"

#include <iostream>

using namespace std;

Engine::Engine(Car & c) : car(c) {
    this->accelerator = 0;
    this->tractionControl = 1;
    this->tractionControlDelta = 0.05;
}

void Engine::setMass(const float mass) {
    this->_mass = mass;
    this->_rpmDelta = 500;
}

void Engine::setRpm(const float maxRpm, const float idleRpm, const float stallRpm, 
        const float startRpm) {
    this->_maxRpm = maxRpm;
    this->_idleRpm = idleRpm;
    this->_stallRpm = stallRpm;
    this->_startRpm = startRpm;

    // Set the current RPM as the start, might need to elaborate this later
    this->_currentRpm = startRpm;
}

void Engine::setRpm(const float rpm) {
    this->_currentRpm = rpm;

    if (this->_currentRpm < this->_idleRpm) {
        this->_currentRpm = this->_idleRpm;
    }
}

void Engine::setDifferential(const float ratio) {
    this->_differentialRatio = ratio;
}

void Engine::setTorqueCurve(const Curve & curve, const float maxTorque) {
    this->_torqueCurve = curve;
    this->_maxTorque = maxTorque;
}

void Engine::print() const {
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

float Engine::getIdleRpm() {
    return this->_idleRpm;
}

float Engine::getDifferential() {
    return this->_differentialRatio;
}

float Engine::calculateTorque() {
    // Our torque equation here is torque * gear ratio * differential ratio * efficiency
    float torque = this->_torqueCurve[this->_currentRpm] * this->_maxTorque;
    return torque * this->_differentialRatio * 0.7;
}

float Engine::calculateTorque(float rpm) {
    // The torque is not defined under the min RPM so we make sure it is at least that
    if (rpm < this->_stallRpm) {
        rpm = 4500;
    }


    // Calculate the traction control needed
    float slip = this->car.maxSlip();
    if (slip > 0.8 && this->tractionControl > 0) {
        this->tractionControl -= this->tractionControlDelta;
    } else if (slip < 0.8 && this->tractionControl < 1) {
        this->tractionControl += this->tractionControlDelta;
    }

    //std::cout << "slip: " << slip << ", traction: " << this->tractionControl << std::endl;

    return this->_torqueCurve[rpm] * this->accelerator ;
}

void Engine::accelerate(float time) {
    this->_currentRpm += this->_rpmDelta * time;

    // make sure the rpm is capped at the maximum
    // TODO: engine explodes
    if (this->_currentRpm > this->_maxRpm) {
        this->_currentRpm = this->_maxRpm;
    }
}

void Engine::decelerate(float time) {
    this->_currentRpm -= this->_rpmDelta * time;

    // The lowest value at the moment is the idle RPM
    if (this->_currentRpm < this->_idleRpm) {
        this->_currentRpm = this->_idleRpm;
    }
}

void Engine::pressAccelerator() {
    this->accelerator = 1.0;
}

void Engine::releaseAccelerator() {
    this->accelerator = 0.0;
}

/******************************************************************************
 * The gearbox
 *****************************************************************************/
Gearbox::Gearbox(Car & c) : car(c) {
    this->_gearRatios = NULL;

    // Set the current gear to neutral
    this->_currentGear = -1;
}

Gearbox::~Gearbox() {
    if (this->_gearRatios != NULL) delete [] this->_gearRatios;
}

Gearbox::Gearbox(const Gearbox & other) : car(other.car) {
    this->_nGears = other._nGears;
    this->_gearRatios = new float[this->_nGears];
    for (int i = 0; i < this->_nGears; ++i) {
        this->_gearRatios[i] = other._gearRatios[i];
    }

    this->_shiftDownRpm = other._shiftDownRpm;
    this->_shiftUpRpm = other._shiftUpRpm;
    this->_currentGear = -1;
}

Gearbox & Gearbox::operator=(const Gearbox & other) {
    throw "Not implemented";
    return *this;
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
    Engine & engine = this->car.getEngine();
    float currentRpm = engine.getCurrentRpm();
    if (this->_currentGear == -1) {
        // If we have the min rpm, shift into first
        if (currentRpm >= engine.getIdleRpm()) {
            this->_currentGear = 1;
        }
    } else if (this->_currentGear == 1) {
        // Check if we need to shift down
        if (currentRpm < engine.getIdleRpm()) {
            this->_currentGear = -1;
        } else if (currentRpm >= this->_shiftUpRpm) {
            this->_shiftUp();
        }
    } else {
        // Check if we need to shift down
        if (currentRpm <= this->_shiftDownRpm) {
            this->_shiftDown();
        } else if (currentRpm >= this->_shiftUpRpm 
                && this->_currentGear < this->_nGears - 1) {
            this->_shiftUp();
        }
    }
}

void Gearbox::_shiftDown() {
    --this->_currentGear;

    // Reset the RPM to the top of the range
    this->car.getEngine().getCurrentRpm() = this->_shiftUpRpm - 100;
}

void Gearbox::_shiftUp() {
    ++this->_currentGear;

    // Reset the RPM to the bottom of the range
    this->car.getEngine().getCurrentRpm() = this->_shiftDownRpm + 100;
}
