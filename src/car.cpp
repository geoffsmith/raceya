#include "car.h"

Car::Car() {
    this->_obj = Obj::makeObj("resources/r8/R8.obj");
}

void Car::render() {
    // Render the various groups in the car obj
    this->_obj->renderGroup("Base");
    this->_obj->renderGroup("BLightL");
    this->_obj->renderGroup("BLightLG");
    this->_obj->renderGroup("BLightR");
    this->_obj->renderGroup("BLightRG");
    this->_obj->renderGroup("Body");
}
