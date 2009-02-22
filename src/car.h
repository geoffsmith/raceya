/**
 * Class representing a car. This includes (so far) its rendering, animation and movement.
 */
#pragma once

#include "obj.h"

class Car {
    public:
        Car();
        void render();

    private:
        Obj * _obj;
};
