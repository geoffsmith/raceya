/**
 * Class representing a car. This includes (so far) its rendering, animation and movement.
 *
 * TODO:
 *  * Rotate the breaks along with wheel, probably best to refactor rendering group into wheel
 *  * Use a turning circle to calculate the car's rotation
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

        // Get a pointer to the player's position
        float * getPosition();

    private:
        Obj * _obj;
        Wheel * _wheels[4];

        // The sideways angle of the wheels for steering animation, this is +/- angle away
        // from front facing
        float _wheelsAngle;

        // The angle away from rest of the steering wheel, - is left, + is right
        float _steeringAngle;
        // How much a keypress changes the steeringAngle
        float _steeringDelta;

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
