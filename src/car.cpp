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

Car::Car() {
    this->_modelScale = 1;

    this->_wheelDiameter = 0.645; // meters;
    this->_wheelsAngle = 0;
    this->_steeringAngle = 0;
    this->_steeringDelta = 5; // degrees
    this->_currentSteering = 0;
    this->_maxSteeringAngle = 60; // degrees

    // Set up the engine
    this->_engineRPM = 0;
    this->_engineMaxRPM = 9000;
    this->_acceleratorPressed = false;
    this->_acceleratorRPMSec = 500;

    // Set up the gears
    this->_finalDriveAxisRatio = 3.46;
    this->_numberOfGears = 6;
    this->_gearRatios = new float[this->_numberOfGears];
    this->_gearRatios[0] = 4.37;
    this->_gearRatios[1] = 2.71;
    this->_gearRatios[2] = 1.88;
    this->_gearRatios[3] = 1.41;
    this->_gearRatios[4] = 1.13;
    this->_gearRatios[5] = 0.93;
    this->_currentGear = 0;
    this->_engineGearUpRPM = 6000;
    this->_engineGearDownRPM = 2000;

    this->_vector[0] = 0;
    this->_vector[1] = 0;
    this->_vector[2] = 0;

    this->_localOrigin[0][0] = -1;
    this->_localOrigin[0][1] = 0;
    this->_localOrigin[0][2] = 0;

    this->_localOrigin[1][0] = 0;
    this->_localOrigin[1][1] = 1;
    this->_localOrigin[1][2] = 0;

    this->_localOrigin[2][0] = 0;
    this->_localOrigin[2][1] = 0;
    this->_localOrigin[2][2] = -1;
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

void Car::_updateEngine() {
    // If the accelerator is pressed we increase the RPM be the accerelation coefficient
    if (this->_acceleratorPressed) {
        this->_engineRPM += this->_acceleratorRPMSec * FrameTimer::timer.getSeconds();
    // ... otherwise we decrease the RPM
    } else if (this->_engineRPM > 0) {
        this->_engineRPM -= (this->_acceleratorRPMSec / 2.0) 
            * FrameTimer::timer.getSeconds();
        // if this is negative, clamp to 0
        if (this->_engineRPM < 0) this->_engineRPM = 0;
    }

    // Check if we need to shift up or down a gear
    if (this->_engineRPM >= this->_engineGearUpRPM && this->_currentGear < this->_numberOfGears - 1) {
        ++this->_currentGear;
        // and reset the RPM to the lower amount
        // TODO, this should probably be more clever and match the speed
        this->_engineRPM = this->_engineGearDownRPM + this->_acceleratorRPMSec;
    }
    
    // Check if we need to shift down a gear, this will only shift down to 1 because 0 is
    // reverse
    if (this->_engineRPM <= this->_engineGearDownRPM && this->_currentGear > 1) {
        --this->_currentGear;
        this->_engineRPM = this->_engineGearUpRPM - this->_acceleratorRPMSec;
    }

}

void Car::_updateLay() {
    float wheelPoint[4][3];
    float groundPoint[4][3];
    // Vector from center to target position
    float ct[3];
    float normal[3];
    float center[3];
    Matrix matrix;

    // For each wheel find the closest point on the ground
    // TODO: do i still need this?
    this->_matrix.multiplyVector(this->_center, center);
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->getGroundContact(wheelPoint[i]);
        // Transform this point to world coords
        this->_matrix.multiplyVector(wheelPoint[i]);

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
    int maxIndex = 0;
    float maxY = 0;
    for (int i = 0; i < 4; ++i) {
        if (i == 0 || maxY < groundPoint[i][1]) {
            maxIndex = i;
            maxY = groundPoint[i][1];
        }
    }

    vertexSub(groundPoint[maxIndex], wheelPoint[maxIndex], ct);
    vertexAdd(ct, this->_position, this->_position);
    vertexAdd(ct, center, center);

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
    // .. make sure the wheel point is above
    float angle3;
    angle3 = -1 * angleInPlaneY(os, ot, this->_vector);

    // * -1 the angle if the lower wheel is the back wheel
    //if (maxIndex <= 1)
    //{
        //angle3 *= -1;
    //}
    
    float yAxis[3];

    // TODO: Investigate nan here
    if (!isnan(angle3)) {
        yAxis[0] = 0;
        yAxis[1] = 1;
        yAxis[2] = 0;

        float xAxis[3];
        xAxis[0] = 1;
        xAxis[1] = 0;
        xAxis[2] = 0;
        
        //if (angle3 > 5) angle3 = 5;
        //if (angle3 < -5) angle3 = -5;

        // Rotate the car vector around normal between vector and y = 1
        crossProduct(this->_vector, yAxis, normal);
        matrix.reset();
        matrix.rotate(angle3, normal);
        matrix.multiplyVector(this->_vector);

        // Rotate the _position of the car around the top wheel
        float oldPosition[3];
        vertexCopy(this->_position, oldPosition);

        matrix.reset();
        matrix.translate(
                -1 * oldPosition[0],
                -1 * oldPosition[1], 
                -1 * oldPosition[2]);
        matrix.translate(
                -1 * wheelPoint[maxIndex][0], 
                -1 * wheelPoint[maxIndex][1], 
                -1 * wheelPoint[maxIndex][2]);

        matrix.rotate(angle3, normal);

        matrix.translate(
                wheelPoint[maxIndex][0], 
                wheelPoint[maxIndex][1], 
                wheelPoint[maxIndex][2]);
        matrix.translate(oldPosition[0], oldPosition[1], oldPosition[2]);
        //matrix.multiplyVector(this->_position);
    }
}

void Car::_updateComponents() {
    // Update the car's steering
    this->_updateSteering();

    // Update the engine
    this->_updateEngine();

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
    this->_calculateMovement();
    float tmp[3];
    vertexMultiply(FrameTimer::timer.getSeconds(), this->_vector, tmp);
    vertexAdd(this->_position, tmp, this->_position);

    // Check for collision with the ground on this new position
    this->_groundCollisionCorrection();

    // Update the car's vector
    /*
    Matrix matrix;
    matrix.rotateY(-1 * this->_steeringAngle * FrameTimer::timer.getSeconds());
    matrix.multiplyVector(this->_vector);
    
    // Update the car position based on how much the wheels have turned
    float wheelCircumferance = PI * this->_wheelDiameter;
    float moveForward = wheelTurns * wheelCircumferance;
    this->_position[0] += moveForward * this->_vector[0];
    this->_position[1] += moveForward * this->_vector[1];
    this->_position[2] += moveForward * this->_vector[2];
    */

    // Update the car's position in relation to the ground
    //this->_updateLay();
}

void Car::_updateMatrix() {
    // The car's angle of rotation, calculated from the _vector
    float zVector[3];
    float yVector[3];
    float xVector[3];

    yVector[0] = 0;
    yVector[1] = 1;
    yVector[2] = 0;

    xVector[0] = 1;
    xVector[1] = 0;
    xVector[2] = 0;

    zVector[0] = 0;
    zVector[1] = 0;
    zVector[2] = -1;

    // Reset the matrix
    this->_matrix.reset();

    // Move the car to its new position
    this->_matrix.translate(this->_position[0], this->_position[1], this->_position[2]);

    Matrix centerMatrix;
    centerMatrix.rotateY(180);
    centerMatrix.scale(this->_modelScale);
    float center[3];

    centerMatrix.multiplyVector(this->_center, center);


    // Translate away from center
    this->_matrix.translate(center[0], center[1], center[2]);
    
    // Set the cars orientation using the quaternion
    Matrix orientationMatrix;
    this->_orientation.toRotationMatrix(orientationMatrix);
    this->_matrix.multiplyMatrix(&orientationMatrix);

    this->_matrix.translate(-1 * center[0], -1 * center[1], -1 * center[2]);


    // Rotate the car to be parallel to ground
    //this->_matrix.rotateZ(180);
    //this->_matrix.rotateX(90);
    this->_matrix.rotateY(180);
    this->_matrix.scale(this->_modelScale);
}

void Car::render() {
    this->_updateMatrix();

    this->_updateComponents();

    glPushMatrix();

    this->_updateMatrix();

    glMultMatrixf(this->_matrix.getMatrix());

    this->_bodyDof->render(true);

    // rotate the front wheel according to the steering
    this->_wheelsAngle = this->_steeringAngle;
    // Update the rotation of the front wheels
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->setAngle(this->_wheelsAngle);
    }

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
                case SDLK_UP:
                    this->_acceleratorPressed = true;
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
                case SDLK_UP:
                    this->_acceleratorPressed = false;
                default:
                    break;
            }
            break;
        default:
            break;
    }
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

void Car::setBody(Dof * dof) {
    this->_bodyDof = dof;
}

void Car::setTrack(Track * track) {
    this->_track = track;

    // Set the start position to the position on the track
    vertexCopy(this->_track->startPosition, this->_position);

    // Add to the position
    //this->_position[1] += 500;
}

void Car::setWheel(Wheel * wheel, int index) {
    this->_wheels[index] = wheel;
}

void Car::setCenter(float * center) {
    vertexCopy(center, this->_center);
}

float * Car::getVector() {
    return this->_localOrigin[0];
}

void Car::setInertia(float * inertia) {
    vertexCopy(inertia, this->_inertia);
}

void Car::setMass(float mass) {
    this->_mass = mass;
}

void Car::setBodyArea(float area) {
    this->_bodyArea = area;
}

void Car::setDragCoefficient(float coefficient) {
    this->_dragCoefficient = coefficient;
}

/******************************************************************************
 * Collision detection
 *****************************************************************************/
bool Car::_isOnGround() {
    float wheelPoint[3];
    float groundPoint[3];
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->getGroundContact(wheelPoint);
        // Transform this point to world coords
        this->_matrix.multiplyVector(wheelPoint);

        // Get the closest point on the track
        findClosestPoint(this->_track->getDofs(), this->_track->getNDofs(), 
                wheelPoint, groundPoint);

        // check for NaN
        // TODO: this is probably caused by a bug in closest point
        if (isnan(groundPoint[0]) 
                || isnan(groundPoint[1]) 
                || isnan(groundPoint[2])) {
            // Set the ground point to the wheel point
            vertexCopy(wheelPoint, groundPoint);
        }

        // If the ground point is >= wheel point, the car is on the ground
        if (groundPoint[1] >= wheelPoint[1]) return true;
    }
    return false;
}

void Car::_groundCollisionCorrection() {
    float wheelPoints[4][3];
    float groundPoints[4][3];
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->getGroundContact(wheelPoints[i]);
        // Transform this point to world coords
        this->_matrix.multiplyVector(wheelPoints[i]);

        // Get the closest point on the track
        findClosestPoint(this->_track->getDofs(), this->_track->getNDofs(), 
                wheelPoints[i], groundPoints[i]);

        // check for NaN
        // TODO: this is probably caused by a bug in closest point
        if (isnan(groundPoints[i][0]) 
                || isnan(groundPoints[i][1]) 
                || isnan(groundPoints[i][2])) {
            // Set the ground point to the wheel point
            vertexCopy(wheelPoints[i], groundPoints[i]);
        }
    }

    // Go through the ground points and find the biggest negative difference
    int largestIndex = 0;
    float maxDifference = 1;
    float difference;
    for (int i = 0; i < 4; ++i) {
        difference = wheelPoints[i][1] - groundPoints[i][1];
        if (i == 0 || (difference < 0 && difference > maxDifference)) {
            maxDifference = difference;
            largestIndex = i;
        }
    }

    // If maxDifference is still 1, all the wheels are above the ground
    if (maxDifference < 0) {
        // The wheel is below the ground surface, we need to move the car up by the
        // difference
        this->_position[1] += -1 * maxDifference;
    }
}

/******************************************************************************
 * Physics stuff
 *****************************************************************************/
void Car::_calculateMovement() {
    // Calculate the force of gravity
    float tmpForce[3];
    float accumulativeForce[] = { 0, 0, 0 };
    float gravityForce[3];
    float yAxis[] = { 0, -9.8, 0 };
    float time = FrameTimer::timer.getSeconds();
    float acceleration[3];
    float dragForce[3];
    vertexMultiply(this->_mass, yAxis, gravityForce);
    vertexAdd(gravityForce, accumulativeForce, accumulativeForce);

    // Add the upwards for equal to gravity
    if (this->_isOnGround()) {
        // If a wheel is on the ground we counter act vertical aspect of the car's
        // vector
        vertexMultiply(-1, gravityForce, gravityForce);
        vertexAdd(gravityForce, accumulativeForce, accumulativeForce);
    }

    // Add the forces for each wheel
    for (int i = 0; i < 4; ++i) {
        if (this->_wheels[i]->isPowered) {
            // TODO check for steering
            // Add a force in the local X direction related to the RPM
            vertexMultiply(this->_engineRPM / 1000 * this->_mass, this->_localOrigin[0], tmpForce);
            vertexAdd(tmpForce, accumulativeForce, accumulativeForce);
        }
    }

    // Add drag
    // NOTE _mass ^ 2 here needs checking, but will do for now...
    float drag = (-1.0 / 391.0) * this->_mass * this->_mass * this->_bodyArea * this->_dragCoefficient;
    vertexMultiply(drag, this->_vector, dragForce);
    vertexAdd(dragForce, accumulativeForce, accumulativeForce);

    // convert sum of forces into acceleration vector
    vertexMultiply(1 / this->_mass, accumulativeForce, acceleration);

    // Scale the force by time
    vertexMultiply(time, acceleration, acceleration);

    // Apply the force to the vector
    vertexAdd(acceleration, this->_vector, this->_vector);
}
