/**
 * Class representing a car. This includes (so far) its rendering, animation and movement.
 */
#pragma once

#include <SDL/SDL.h>
#include "obj.h"
#include "wheel.h"

class Car {
    public:
        Car();
        void render();

        // Handle key presses
        void handleKeyPress(SDL_Event &event);

    private:
        Obj * _obj;
        Wheel * _wheels[4];

        float _engineRPM;
        float _engineMaxRPM;

        float _finalDriveAxisRatio;
        float _gearRatios[6];
        int _currentGear;

        // Update the moving components of the car such as engine and wheels
        void _updateComponents();
};
