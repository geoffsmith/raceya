/**
 * Class representing a wheel.
 */
#pragma once

#include "obj.h"
#include "matrix.h"

class Wheel {
    public:
        Wheel(int position, Obj * obj);
        void render();
        void turn();

    private:
        float _centerVertex[3];
        float _rotation;
        int _position;
        Obj * _obj;
        float _wheelCenter[3];

};
