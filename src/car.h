/**
 * Class representing a car. This includes (so far) its rendering, animation and movement.
 */
#pragma once

#include "obj.h"
#include "wheel.h"

class Car {
    public:
        Car();
        void render();

    private:
        Obj * _obj;
        Wheel * _wheels[4];
};
