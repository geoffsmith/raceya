/**
 * Simple camera class. Follows the player's car and the rotation around the car can be
 * modified. 
 * 
 * More useful for development than anything else at the moment. A different camera
 * which followed the player from the back and has a rotational delay will be better for 
 * gameplay.
 */
#include "camera.h"
#include "lib.h"
#include "vector.h"

#include <OpenGL/gl.h>
#include <iostream>
#include <math.h>

using namespace std;

Camera::Camera(Car * playersCar) {
    this->_playersCar = playersCar;

    // Initialise the two rotations and distance
    this->_distance = 7;
    this->_rotationY = 30;
    this->_rotationDelta = 10;

    // Initalise yaw
    this->_targetYawAngle = 0;
    this->_currentYawAngle = 180;
    this->_maxYawMovementPerFrame = 2;
}

void Camera::_calculateYawAngle() {
    // First find the angle between the car direction and the X unit vector on the X/Z
    // plane
    float zUnit[] = { 0, 0, -1 };
    float xUnit[] = { -1, 0, 0 };
    float carXAxis[3];
    float carAngle;
    float targetAngle;
    float workingAngle;

    vector<float> v;
    this->_playersCar->getVector(v);
    carXAxis[0] = v[0];
    carXAxis[1] = v[1];
    carXAxis[2] = v[2];

    carAngle = angleInPlane(xUnit, carXAxis, zUnit, xUnit);

    // ... then add the target angle
    targetAngle = carAngle + this->_targetYawAngle;

    // ... get the difference between the current and target angle
    workingAngle = this->_currentYawAngle - targetAngle;

    // We only want to move +/- 180, otherwise, we might try to go the long way round
    if (workingAngle > 180) {
        workingAngle -= 360;
    } else if (workingAngle < -180) {
        workingAngle += 360;
    }

    // ... clamp to maximum
    if (fabs(workingAngle) > this->_maxYawMovementPerFrame) {
        workingAngle = this->_maxYawMovementPerFrame * fabs(workingAngle) / workingAngle;
    }

    // ... move it
    this->_currentYawAngle -= workingAngle;
}

void Camera::viewTransform() {
    // Calculate the new yaw angle
    this->_calculateYawAngle();

    // Rotate the scene for the camera
    glTranslatef(0, 0, -1 * this->_distance);
    glRotatef(this->_rotationY, 1, 0, 0);
    glRotatef(this->_currentYawAngle - 90, 0, 1, 0);

    // Translate so that the player's car is the focus
    Vector playerPosition = this->_playersCar->getPosition();
    glTranslatef(-1 * playerPosition[0], -1 * playerPosition[1], -1 * playerPosition[2]);
}

void Camera::handleKeyPress(SDL_Event &event) {
    switch (event.type) {
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_j:
                    this->_targetYawAngle -= this->_rotationDelta;
                    break;
                case SDLK_l:
                    this->_targetYawAngle += this->_rotationDelta;
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
