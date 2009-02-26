/**
 * Simple camera class. Follows the player's car and the rotation around the car can be
 * modified. 
 * 
 * More useful for development than anything else at the moment. A different camera
 * which followed the player from the back and has a rotational delay will be better for 
 * gameplay.
 */
#include "camera.h"
#include <OpenGL/gl.h>


Camera::Camera(Car * playersCar) {
    this->_playersCar = playersCar;

    // Initialise the two rotations and distance
    this->_distance = 7;
    this->_rotationX = -80;
    this->_rotationY = 30;
    this->_rotationDelta = 10;
}

void Camera::viewTransform() {

    // Rotate the scene for the camera
    glTranslatef(0, 0, -1 * this->_distance);
    glRotatef(this->_rotationY, 1, 0, 0);
    glRotatef(this->_rotationX, 0, 1, 0);

    // Translate so that the player's car is the focus
    float * playerPosition = this->_playersCar->getPosition();
    glTranslatef(-1 * playerPosition[0], -1 * playerPosition[1], -1 * playerPosition[2]);
}

void Camera::handleKeyPress(SDL_Event &event) {
    switch (event.type) {
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_j:
                    this->_rotationX -= this->_rotationDelta;
                    break;
                case SDLK_l:
                    this->_rotationX += this->_rotationDelta;
                    break;
                case SDLK_i:
                    this->_rotationY += this->_rotationDelta;
                    break;
                case SDLK_k:
                    this->_rotationY -= this->_rotationDelta;
                    break;
                case SDLK_o:
                    this->_distance -= 1;
                    break;
                case SDLK_u:
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
