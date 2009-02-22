#include "camera.h"
#include <OpenGL/gl.h>

Camera::Camera() {
    // Initialise the two rotations and distance
    this->_distance = 10;
    this->_rotationX = 0;
    this->_rotationY = 0;
    this->_rotationDelta = 10;
}

void Camera::viewTransform() {
    // Rotate the scene for the camera
    glTranslatef(0, 0, -1 * this->_distance);
    glRotatef(this->_rotationX, 1, 0, 0);
    glRotatef(this->_rotationY, 0, 1, 0);
}

void Camera::handleKeyPress(SDL_Event &event) {
    switch (event.type) {
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                    this->_rotationX -= this->_rotationDelta;
                    break;
                case SDLK_RIGHT:
                    this->_rotationX += this->_rotationDelta;
                    break;
                case SDLK_UP:
                    this->_rotationY += this->_rotationDelta;
                    break;
                case SDLK_DOWN:
                    this->_rotationY -= this->_rotationDelta;
                    break;
                case SDLK_q:
                    this->_distance -= 1;
                    break;
                case SDLK_a:
                    this->_distance += 1;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}
