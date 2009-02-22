/**
 * Class representing a user controller camera. Keyboard commands are used to move the
 * camera around the scene.
 */
#pragma once

#include <SDL/SDL.h>

class Camera {
    public:
        Camera();

        // Transform the scene to a view from the camera
        void viewTransform();

        // Handle any keypresses which are relevant to the camera
        void handleKeyPress(SDL_Event &event);

    private:
        // At the moment the camera moves around the origin, so we need distance an rotations
        float _distance;
        float _rotationX;
        float _rotationY;
        float _rotationDelta;
};
