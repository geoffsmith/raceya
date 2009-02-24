#include "car.h"
#include "matrix.h"
#include "lib.h"
#include "frame_timer.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <iostream>
#include <math.h>

#define PI 3.14159265

using namespace std;

Car::Car() {
    this->_obj = Obj::makeObj("resources/r8/R8.obj");
    //this->_modelScale = 1248.0312;
    //this->_modelScale = 1.2480312;
    this->_modelScale = 1;

    this->_wheels[0] = new Wheel(0, this->_obj);
    this->_wheels[1] = new Wheel(1, this->_obj);
    this->_wheels[2] = new Wheel(2, this->_obj);
    this->_wheels[3] = new Wheel(3, this->_obj);
    this->_wheelDiameter = 0.645; // m;
    this->_wheelsAngle = 0;
    this->_steeringAngle = 0;
    this->_steeringDelta = 2;
    this->_currentSteering = 0;

    // Set up the engine
    this->_engineRPM = 0;
    this->_engineMaxRPM = 9000;

    // Set up the gears
    this->_finalDriveAxisRatio = 3.46;
    this->_gearRatios[0] = 4.37;
    this->_gearRatios[1] = 2.71;
    this->_gearRatios[2] = 1.88;
    this->_gearRatios[3] = 1.41;
    this->_gearRatios[4] = 1.13;
    this->_gearRatios[5] = 0.93;
    this->_currentGear = 0;

    this->_vector[0] = 0;
    this->_vector[1] = 0;
    this->_vector[2] = -1;
}

void Car::_updateComponents() {
    // Update the car's steering
    this->_steeringAngle += this->_currentSteering * this->_steeringDelta;

    // Find out how much the engine has turned
    float engineTurns = this->_engineRPM * FrameTimer::timer.getMinutes();
    // And divide by current gear ratio
    float wheelTurns = engineTurns / this->_gearRatios[this->_currentGear];
    // ... and divide by the final drive axis ratio
    wheelTurns = wheelTurns / this->_finalDriveAxisRatio;
    // ... and turn the wheels
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->turn(wheelTurns * 360.0);
    }

    // Update the car's vector
    Matrix matrix;
    matrix.rotateY(-1 * this->_steeringAngle * FrameTimer::timer.getSeconds());
    matrix.multiplyVector(this->_vector);
    
    // Update the car position based on how much the wheels have turned
    float wheelCircumferance = PI * this->_wheelDiameter;
    float moveForward = wheelTurns * wheelCircumferance;
    this->_position[0] += moveForward * this->_vector[0];
    this->_position[1] += moveForward * this->_vector[1];
    this->_position[2] += moveForward * this->_vector[2];
}

void Car::render() {
    // The car's angle of rotation, calculated from the _vector
    float angle;
    float zVector[3];

    this->_updateComponents();

    glPushMatrix();
    // Move the car to its new position
    glTranslatef(this->_position[0], this->_position[1], this->_position[2]);

    // Rotate the car to point in the right direction
    zVector[0] = 0;
    zVector[1] = 0;
    zVector[2] = -1;
    angle = -1 * angleBetweenVectors(this->_vector, zVector);
    // We need to establish the direction so we use the x-component of the vector
    if (this->_vector[0] != 0) {
        glRotatef(angle, 0, this->_vector[0], 0);
    } else {
        glRotatef(angle, 0, 1, 0);
    }

    // Rotate the car to be parallel to ground
    glRotatef(180, 0, 0, 1);
    glRotatef(90, 1, 0, 0);
    glScalef(this->_modelScale, this->_modelScale, this->_modelScale);

    // Render the various groups in the car obj
    this->_obj->renderGroup("LicenseF");
    this->_obj->renderGroup("LicenseR");
    this->_obj->renderGroup("Base");
    this->_obj->renderGroup("BLightL");
    this->_obj->renderGroup("BLightLG");
    this->_obj->renderGroup("BLightR");
    this->_obj->renderGroup("BLightRG");
    this->_obj->renderGroup("Body");
    this->_obj->renderGroup("BumperF");
    this->_obj->renderGroup("BumperFB");
    this->_obj->renderGroup("BumperR");
    this->_obj->renderGroup("BumperRB");
    this->_obj->renderGroup("DoorL");
    this->_obj->renderGroup("DoorLine");
    this->_obj->renderGroup("DoorR");
    this->_obj->renderGroup("Driver");
    this->_obj->renderGroup("Exhaust");
    this->_obj->renderGroup("HiInt");
    this->_obj->renderGroup("HLightL");
    this->_obj->renderGroup("HLightLG");
    this->_obj->renderGroup("HLightR");
    this->_obj->renderGroup("HLightRG");
    this->_obj->renderGroup("Hood");
    this->_obj->renderGroup("Interior");
    this->_obj->renderGroup("MirrorR");
    this->_obj->renderGroup("MirrorL");
    this->_obj->renderGroup("Roof");
    this->_obj->renderGroup("Skirt");
    this->_obj->renderGroup("WindF");
    this->_obj->renderGroup("WindFL");
    this->_obj->renderGroup("WindFR");
    this->_obj->renderGroup("WindR");
    this->_obj->renderGroup("WindRL");
    this->_obj->renderGroup("WindRR");
    this->_obj->renderGroup("CaliperFR");
    this->_obj->renderGroup("CaliperFL");
    this->_obj->renderGroup("CaliperRR");
    this->_obj->renderGroup("CaliperRL");

    // rotate the front wheel according to the steering
    this->_wheelsAngle = this->_steeringAngle;
    // Update the rotation of the front wheels
    this->_wheels[2]->setAngle(this->_wheelsAngle);
    this->_wheels[3]->setAngle(this->_wheelsAngle);

    // Render the wheels
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->render();
    }

    glPopMatrix();
}

void Car::handleKeyPress(SDL_Event &event) {
    switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                // If the key is down, we increase the steering angle
                case SDLK_LEFT:
                    this->_currentSteering = -1;
                    break;
                case SDLK_RIGHT:
                    this->_currentSteering = +1;
                    break;
                default:
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                // If the steering key goes up,  we move back to the center
                case SDLK_LEFT:
                    this->_currentSteering = 0;
                    this->_steeringAngle = 0;
                    break;
                case SDLK_RIGHT:
                    this->_currentSteering = 0;
                    this->_steeringAngle = 0;
                    break;
                case SDLK_z:
                    this->_engineRPM += 100;
                    break;
                case SDLK_x:
                    this->_engineRPM -= 100;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

/****************************************************************************************
 * Getters / setters
 ***************************************************************************************/
float * Car::getPosition() {
    return this->_position;
}
