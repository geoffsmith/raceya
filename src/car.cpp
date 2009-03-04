#include "car.h"
#include "matrix.h"
#include "lib.h"
#include "frame_timer.h"
#include "closest_point.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <iostream>
#include <math.h>

#define PI 3.14159265

using namespace std;

Car::Car(Track * track) {
    this->_track = track;

    this->_obj = Obj::makeObj("resources/r8/R8.obj");
    this->_modelScale = 1;

    this->_wheels[0] = new Wheel(0, this->_obj);
    this->_wheels[1] = new Wheel(1, this->_obj);
    this->_wheels[2] = new Wheel(2, this->_obj);
    this->_wheels[3] = new Wheel(3, this->_obj);
    this->_wheelDiameter = 0.645; // meters;
    this->_wheelsAngle = 0;
    this->_steeringAngle = 0;
    this->_steeringDelta = 5; // degrees
    this->_currentSteering = 0;
    this->_maxSteeringAngle = 60; // degrees

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

    // Calculate the center of the object
    this->_calculateCenter();
}

void Car::_updateSteering() {
    // if the _currentSteering is 0, we want to bear back towards a steering of 0
    float direction = fabs(this->_steeringAngle) != this->_steeringAngle ? 1 : -1;
    if (this->_currentSteering == 0) {
        // make sure there isn't any wobble
        if (fabs(this->_steeringAngle) <= this->_steeringDelta) {
            this->_steeringAngle = 0;
        } else if (this->_steeringAngle != 0) {
            this->_steeringAngle +=  direction * this->_steeringDelta * 2;
        }
    // Otherwise we turn in the direction set by the player
    } else {
        this->_steeringAngle += this->_currentSteering * this->_steeringDelta;
    }
    
    // If the steering has gone too far, we clamp it
    if (fabs(this->_steeringAngle) > this->_maxSteeringAngle) {
        this->_steeringAngle = -1 * direction * this->_maxSteeringAngle;
    }
}

void Car::_updateLay() {
    float wheelPoint[4][3];
    float groundPoint[4][3];
    // Vector from center to target position
    float ct[3];
    // vector from center to current position
    float cp[3];
    float angle;
    float normal[3];
    float center[3];
    float distance;
    Matrix matrix;

    // For each wheel find the closest point on the ground
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->getGroundContact(wheelPoint[i]);
        // Transform this point to world coords
        this->_matrix.multiplyVector(wheelPoint[i]);
        this->_matrix.multiplyVector(this->_center, center);

        // Get the closest point on the track
        findClosestPoint(this->_track->getDofs(), this->_track->getNDofs(), 
                wheelPoint[i], groundPoint[i]);

        // check for NaN
        // TODO: this is probably caused by a bug in closest point
        if (isnan(groundPoint[i][0]) 
                || isnan(groundPoint[i][1]) 
                || isnan(groundPoint[i][2])) {
            // Set the ground point to the wheel point
            vertexCopy(wheelPoint[i], groundPoint[i]);
        }
    }

    // Move the car to the highest point
    int maxIndex;
    float maxY;
    for (int i = 0; i < 4; ++i) {
        if (i == 0 || maxY < groundPoint[i][1]) {
            maxIndex = i;
            maxY = groundPoint[i][1];
        }
    }

    vertexSub(groundPoint[maxIndex], wheelPoint[maxIndex], ct);
    vertexAdd(ct, this->_position, this->_position);

    // We also need to add this to each wheel position
    for (int i = 0; i < 4; ++i) {
        vertexAdd(ct, wheelPoint[i], wheelPoint[i]);
    }


    float os[3];
    float ot[3];
    // Find angle to rotate the car around the axis pointing forward through
    // the bottom of the wheel just found, we do this around the the topmost of
    // the two wheels on the opposite side of the car to the wheel just found 
    // i.e. left -> right, right -> left
    // maxIndex % 2 == 0 -> original wheel is on right, we want left
    /*int maxIndex2;
    if (maxIndex % 2 == 0) {
        if (groundPoint[1][1] > groundPoint[3][1]) {
            maxIndex2 = 1;
        } else {
            maxIndex2 = 3;
        }
    } else {
        if (groundPoint[0][1] > groundPoint[2][1]) {
            maxIndex2 = 0;
        } else {
            maxIndex2 = 2;
        }
    }

    // Find angle between this new point, the highest and this new groundPoint
    vertexSub(wheelPoint[maxIndex2], groundPoint[maxIndex], os);
    vertexSub(groundPoint[maxIndex2], groundPoint[maxIndex], ot);
    float angle2 = angleBetweenVectors(os, ot);*/

    // Adjust the roll of the car
    // TODO: we don't have a roll variable yet

    // Now we do the same with the pitch of the car, this time front->back, 
    // back -> front
    int maxIndex3;
    if (maxIndex < 2) {
        if (groundPoint[2][1] > groundPoint[3][1]) {
            maxIndex3 = 2;
        } else {
            maxIndex3 = 3;
        }
    } else {
        if (groundPoint[0][1] > groundPoint[1][1]) {
            maxIndex3 = 0;
        } else {
            maxIndex3 = 1;
        }
    }

    // When the car is far away from an object, some of these vectors might be 0
    vertexSub(wheelPoint[maxIndex3], groundPoint[maxIndex], os);
    vertexSub(groundPoint[maxIndex3], groundPoint[maxIndex], ot);
    
    // Get the angle between these vectors in the plane in the direction of _vector
    float angle3 = angleBetweenVectors(os, ot);
    float yAxis[3];

    // TODO: Investigate why this angle is sometimes nan
    if (!isnan(angle3)) {
        yAxis[0] = 0;
        yAxis[1] = 1;
        yAxis[2] = 0;

        // flip the sign of the angle if ground is above the wheel
        if (groundPoint[maxIndex3][1] > wheelPoint[maxIndex3][1]) {
            angle3 *= -1;
        }

        cout << "Angle 3: " << angle3 << endl;

        // Rotate the car vector around normal between vector and y = 1
        crossProduct(this->_vector, yAxis, normal);
        matrix.reset();
        matrix.rotate(angle3, normal);
        matrix.multiplyVector(this->_vector);

        // Rotate the _position of the car around the top wheel
        float oldPosition[3];
        vertexCopy(this->_position, oldPosition);

        matrix.reset();
        matrix.translate(oldPosition[0], oldPosition[1], oldPosition[2]);
        matrix.translate(
                -1 * wheelPoint[maxIndex][0], 
                -1 * wheelPoint[maxIndex][1], 
                -1 * wheelPoint[maxIndex][2]);

        matrix.rotate(angle3, normal);
        //matrix.multiplyVector(this->_position);

        matrix.translate(
                wheelPoint[maxIndex][0], 
                wheelPoint[maxIndex][1], 
                wheelPoint[maxIndex][2]);
        matrix.translate(
                -1 * oldPosition[0],
                -1 * oldPosition[1], 
                -1 * oldPosition[2]);
    }
}

void Car::_updateComponents() {
    // Update the car's steering
    this->_updateSteering();

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

    // Update the car's position in relation to the ground
    this->_updateLay();
}

void Car::_updateMatrix() {
    // The car's angle of rotation, calculated from the _vector
    float angle;
    float zVector[3];
    float yVector[3];
    float xVector[3];

    yVector[0] = 0;
    yVector[1] = 1;
    yVector[2] = 0;

    xVector[0] = 1;
    xVector[1] = 0;
    xVector[2] = 0;

    // Reset the matrix
    this->_matrix.reset();

    // Move the car to its new position
    this->_matrix.translate(this->_position[0], this->_position[1], this->_position[2]);

    Matrix centerMatrix;
    centerMatrix.rotateZ(180);
    centerMatrix.rotateX(90);
    centerMatrix.scale(this->_modelScale);
    float center[3];

    centerMatrix.multiplyVector(this->_center, center);


    // Translate away from center
    this->_matrix.translate(-1 * center[0], -1 * center[1], -1 * center[2]);
    
    // Adjust the pitch of the car
    float n[3];
    crossProduct(yVector, this->_vector, n);
    angle = 90 - angleBetweenVectors(this->_vector, yVector);
    if (this->_vector[2] != 0) angle *= -1 * fabs(this->_vector[2]) / this->_vector[2];
    cout << "Angle: " << angle << endl;
    this->_matrix.rotate(angle, n);

    // Rotate the car to point in the right direction
    zVector[0] = 0;
    zVector[1] = 0;
    zVector[2] = -1;
    angle = -1 * angleBetweenVectors(this->_vector, zVector);
    // We need to establish the direction so we use the x-component of the vector
    // We use the sign of _vector to direct the rotation, the angle is always
    // 0 <= angle <= 180
    if (this->_vector[0] != 0) {
        this->_matrix.rotate(angle * (fabs(this->_vector[0]) / this->_vector[0]), yVector);
    } else {
        this->_matrix.rotate(angle, yVector);
    }


    this->_matrix.translate(center[0], center[1], center[2]);

    //cout << "V: " << this->_vector[0] << ", " << this->_vector[1] << ", " << this->_vector[2] << endl;

    // Rotate the car to be parallel to ground
    this->_matrix.rotateZ(180);
    this->_matrix.rotateX(90);
    this->_matrix.scale(this->_modelScale);
}

void Car::render() {
    this->_updateMatrix();

    this->_updateComponents();

    glPushMatrix();

    this->_updateMatrix();

    glMultMatrixf(this->_matrix.getMatrix());

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
                    this->_currentSteering -= 1;
                    break;
                case SDLK_RIGHT:
                    this->_currentSteering += 1;
                    break;
                default:
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                // If the steering key goes up,  we move back to the center
                case SDLK_LEFT:
                    this->_currentSteering += 1;
                    break;
                case SDLK_RIGHT:
                    this->_currentSteering -= 1;
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

void Car::_calculateCenter() {
    float bounds[6];
    Matrix matrix;

    // Find the bounds
    this->_obj->calculateBounds(&matrix, bounds);

    // And calculate the center
    this->_center[0] = (bounds[0] + bounds[1]) / 2.0;
    this->_center[1] = (bounds[2] + bounds[3]) / 2.0;
    this->_center[2] = (bounds[4] + bounds[5]) / 2.0;
}

/*******************************************************************************
 * Getters / setters
 ******************************************************************************/
float * Car::getPosition() {
    return this->_position;
}

float Car::getRPM() {
    return this->_engineRPM;
}

float * Car::getWheelPosition() {
    float * result = new float[3];
    result[0] = 0.953;
    result[1] = 1.434;
    result[2] = 0.342;
    // transform the vertex
    this->_matrix.multiplyVector(result);
    return result;
}
