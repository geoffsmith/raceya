#include "drive_systems.h"

Engine::Engine(float mass, float maxRpm, float idleRpm, float stallRpm, float startRpm) {
}

void Engine::setMass(float mass) {
    this->_mass = mass;
}

void Engine::setRpm(float maxRpm, float idleRpm, float stallRpm, float startRpm) {
    this->_maxRpm = maxRpm;
    this->_idleRpm = idleRpm;
    this->_stallRpm = stallRpm;
    this->_startRpm = startRpm;
}

void Engine::setDifferential(float ratio) {
    this->_differentialRatio = ratio;
}
