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

        // Model scale -> puts the model into meters's
        float _modelScale;

        // Engine related variables
        float _engineRPM;
        float _engineMaxRPM;

        // Gear related variables
        float _finalDriveAxisRatio;
        float _gearRatios[6];
        int _currentGear;
        float _wheelDiameter;

        // Position
        float _position[3];
        float _vector[3];

        // Update the moving components of the car such as engine and wheels
        void _updateComponents();
};
