/**
 * Class representing a car. This includes (so far) its rendering, animation and movement.
 *
 * TODO:
 *  * Use a turning circle to calculate the car's rotation
 *  * Clean up memory, this will be important when there are opponents
 */
#pragma once

#include <SDL/SDL.h>
#include "wheel.h"
#include "track.h"
#include "dof.h"

class Dof;

class Car {
    public:
        Car();
        void render();

        // Handle key presses
        void handleKeyPress(SDL_Event &event);

        // Get a pointer to the player's position
        float * getPosition();

        float getRPM();

        float * getWheelPosition();

        // Setters
        void setBody(Dof * dof);
        void setTrack(Track * track);
        void setWheel(Wheel * wheel, int index);
        void setCenter(float * center);

    private:
        Wheel * _wheels[4];

        // Needs a reference to the track to do collision detection
        Track * _track;

        // The sideways angle of the wheels for steering animation, this is +/- angle away
        // from front facing
        float _wheelsAngle;

        // The angle away from rest of the steering wheel, - is left, + is right
        float _steeringAngle;
        // How much a keypress changes the steeringAngle
        float _steeringDelta;
        float _currentSteering;
        float _maxSteeringAngle;

        // Model scale -> puts the model into meters's
        float _modelScale;

        // Engine related variables
        float _engineRPM;
        float _engineMaxRPM;
        bool _acceleratorPressed;
        float _acceleratorRPMSec;

        // Gear related variables
        float _finalDriveAxisRatio;
        float * _gearRatios;
        int _numberOfGears;
        int _currentGear;
        float _wheelDiameter;
        float _engineGearUpRPM;
        float _engineGearDownRPM;

        // Position
        float _position[3];
        float _vector[3];

        // Center of the car
        float _center[3];

        // Update the engine variables
        void _updateEngine();
        // Update the moving components of the car such as engine and wheels
        void _updateComponents();
        // Update the car's steering
        void _updateSteering();
        // Update the car's position so that it lies on the ground
        void _updateLay();
        // Update hte transformation matrix
        void _updateMatrix();

        // the car's transformation matrix
        Matrix _matrix;

        Dof * _bodyDof;
};
