#include "car.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <iostream>

using namespace std;

Car::Car() {
    this->_obj = Obj::makeObj("resources/r8/R8.obj");
    this->_wheels[0] = new Wheel(0, this->_obj);
    this->_wheels[1] = new Wheel(1, this->_obj);
    this->_wheels[2] = new Wheel(2, this->_obj);
    this->_wheels[3] = new Wheel(3, this->_obj);
}

void Car::render() {
    // Render the various groups in the car obj
    this->_obj->renderGroup("LicenseF");
    this->_obj->renderGroup("LicenseR");
    this->_obj->renderGroup("Base");
    this->_obj->renderGroup("BLightL");
    this->_obj->renderGroup("BLightLG");
    this->_obj->renderGroup("BLightR");
    this->_obj->renderGroup("BLightRG");
    this->_obj->renderGroup("Body");
    this->_obj->renderGroup("BumperF");
    this->_obj->renderGroup("BumperFB");
    this->_obj->renderGroup("BumperR");
    this->_obj->renderGroup("BumperRB");
    this->_obj->renderGroup("DoorL");
    this->_obj->renderGroup("DoorLine");
    this->_obj->renderGroup("DoorR");
    this->_obj->renderGroup("Driver");
    this->_obj->renderGroup("Exhaust");
    this->_obj->renderGroup("HiInt");
    this->_obj->renderGroup("HLightL");
    this->_obj->renderGroup("HLightLG");
    this->_obj->renderGroup("HLightR");
    this->_obj->renderGroup("HLightRG");
    this->_obj->renderGroup("Hood");
    this->_obj->renderGroup("Interior");
    this->_obj->renderGroup("MirrorR");
    this->_obj->renderGroup("MirrorL");
    this->_obj->renderGroup("Roof");
    this->_obj->renderGroup("Skirt");
    this->_obj->renderGroup("WindF");
    this->_obj->renderGroup("WindFL");
    this->_obj->renderGroup("WindFR");
    this->_obj->renderGroup("WindR");
    this->_obj->renderGroup("WindRL");
    this->_obj->renderGroup("WindRR");
    this->_obj->renderGroup("CaliperFR");
    this->_obj->renderGroup("CaliperFL");
    this->_obj->renderGroup("CaliperRR");
    this->_obj->renderGroup("CaliperRL");

    // Render the wheels
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->render();
        this->_wheels[i]->turn();
    }
}
