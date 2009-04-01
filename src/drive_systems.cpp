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
    return this->_currentRpm;
}

float Engine::calculateTorque() {
    // Our torque equation here is torque * gear ratio * differential ratio * efficiency
    float torque = this->_torqueCurve[this->_currentRpm] * this->_maxTorque;
    return torque * 3.3 * this->_differentialRatio * 0.7;
}
