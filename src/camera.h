/**
 * Class representing a user controller camera. Keyboard commands are used to move the
 * camera around the scene.
 */
#pragma once

#include <SDL/SDL.h>
#include "car.h"

class Camera {
    public:
        Camera(Car * playersCar);

        // Transform the scene to a view from the camera
        void viewTransform();

        // Handle any keypresses which are relevant to the camera
        void handleKeyPress(SDL_Event &event);

    private:
        // At the moment the camera moves around the origin, so we need distance an rotations
        float _distance;
        float _rotationY;
        float _rotationDelta;

        // The target yaw angle from the car yaw
        float _targetYawAngle;
        float _targetYawModifier;
        float _maxYawMovementPerFrame;
        float _currentYawAngle;
        void _calculateYawAngle();

        // The player's car, so we can follow it
        Car * _playersCar;
};
