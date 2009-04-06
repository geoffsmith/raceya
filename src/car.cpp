#include "car.h"
#include "matrix.h"
#include "lib.h"
#include "frame_timer.h"
#include "closest_point.h"

#include <SDL/SDL.h>
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

    this->_linearVelocity[0] = 0;
    this->_linearVelocity[1] = 0;
    this->_linearVelocity[2] = 0;

    this->_localOrigin[0][0] = -1;
    this->_localOrigin[0][1] = 0;
    this->_localOrigin[0][2] = 0;

    this->_localOrigin[1][0] = 0;
    this->_localOrigin[1][1] = 1;
    this->_localOrigin[1][2] = 0;

    this->_localOrigin[2][0] = 0;
    this->_localOrigin[2][1] = 0;
    this->_localOrigin[2][2] = -1;

    this->_inertiaTensor = Matrix(3);
    this->_inverseInertiaTensor = Matrix(3);

    this->_angularVelocity[0] = 0;
    this->_angularVelocity[1] = 0;
    this->_angularVelocity[2] = 0;

    float euler[] = { PI / 3.0, PI / 8.0, 0 };
    this->_orientation.fromEuler(euler);

    // Try and run the physics at 100Hz
    this->_timer = new FrameTimer(300);
}

Car::~Car() {
    delete this->_timer;
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
        //this->_engineRPM += this->_acceleratorRPMSec * this->_timer->getSeconds();
        this->_engine.getCurrentRpm() += this->_acceleratorRPMSec * this->_timer->getSeconds();
    // ... otherwise we decrease the RPM
    } else if (this->_engineRPM > 0) {
        //this->_engineRPM -= (this->_acceleratorRPMSec / 2.0) 
            //* this->_timer->getSeconds();
        this->_engine.getCurrentRpm() -= (this->_acceleratorRPMSec / 2.0) 
            * this->_timer->getSeconds();
        // if this is negative, clamp to 0
        if (this->_engineRPM < 0) this->_engine.getCurrentRpm() = 0;
    }

    // Check if we need to shift up or down a gear
    /*
    if (this->_engineRPM >= this->_engineGearUpRPM 
            && this->_currentGear < this->_numberOfGears - 1) {
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
    */

}

void Car::_updateComponents() {
    this->_updateMatrix();

    // Update the car's steering
    this->_updateSteering();

    // Update the engine
    this->_updateEngine();

    // Find out how much the engine has turned
    float engineTurns = this->_engine.getCurrentRpm() * this->_timer->getSeconds() * 60.0;
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
    vertexMultiply(this->_timer->getSeconds(), this->_linearVelocity, tmp);
    vertexAdd(this->_position, tmp, this->_position);

    // Check for collision with the ground on this new position
    this->_groundCollisionCorrection();

    this->_updateMatrix();
}

void * Car::update(void * _car) {
    Car * car = (Car *)_car;
    int i = 0;
    float d;
    while (true) {
        // Every 10th time, update the ground point
        if (i % 10 == 0) {
            car->_updateGroundPoints();
            i = 0;
        }
        ++i;

        car->_timer->newFrame();
        car->_updateComponents();
        d = car->_timer->getTimeTillNext();
        SDL_Delay(d);
    }
}

void Car::_updateMatrix() {
    // Reset the matrix
    this->_matrix.reset();

    // Move the car to its new position
    this->_matrix.translate(this->_position[0], this->_position[1], this->_position[2]);

    Matrix centerMatrix;
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
    this->_matrix.scale(this->_modelScale);
}

void Car::render() {

    //this->_updateComponents();

    glPushMatrix();

    this->_updateMatrix();

    glMultMatrixf(this->_matrix.getMatrix());

    this->_bodyDof->render(true);

    // rotate the front wheel according to the steering
    this->_wheelsAngle = this->_steeringAngle;
    // Update the rotation of the front wheels
    for (int i = 0; i < 4; ++i) {
        if (this->_wheels[i]->isSteering()) {
            this->_wheels[i]->setAngle(this->_wheelsAngle);
        }
    }

    // Render the wheels
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->render();

        glPushMatrix();
        GLUquadric * quad = gluNewQuadric();
        glTranslatef(this->_wheelVectors[i][0], this->_wheelVectors[i][1], this->_wheelVectors[i][2]);
        gluSphere(quad, 0.25, 5, 5);
        gluDeleteQuadric(quad);
        glPopMatrix();
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
    return this->_engine.getCurrentRpm();
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
    this->_position[1] += 4;
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
    this->_calculateInertiaTensor();
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

void Car::setEngine(Engine & engine) {
    this->_engine = engine;
}

/******************************************************************************
 * Collision detection
 *****************************************************************************/
void Car::_updateGroundPoints() {
    float wheelPoint[3];
    float groundPoint[3];
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->getGroundContact(wheelPoint);

        // Transform this point to world coords
        this->_matrix.multiplyVector(wheelPoint);

        // Get the closest point on the track
        findClosestPoint(this->_track->getDofs(), this->_track->getNDofs(), 
                wheelPoint, groundPoint);

        if (isnan(groundPoint[0]) 
                || isnan(groundPoint[1]) 
                || isnan(groundPoint[2])) {
            // Set the ground point to the wheel point
            vertexCopy(wheelPoint, groundPoint);
        }
        this->_closestGroundPoints[i] = Vector(groundPoint, 3);
    }
}

void Car::_groundCollisionCorrection() {
    float wheelPoints[4][3];
    float groundPoints[4][3];
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->getGroundContact(wheelPoints[i]);
        this->_matrix.multiplyVector(wheelPoints[i]);
        groundPoints[i][0] = this->_closestGroundPoints[i][0];
        groundPoints[i][1] = this->_closestGroundPoints[i][1];
        groundPoints[i][2] = this->_closestGroundPoints[i][2];
    }

    // Go through the ground points and find the biggest negative difference
    int largestIndex = 0;
    float maxDifference = 1;
    float difference;
    for (int i = 0; i < 4; ++i) {
        difference = wheelPoints[i][1] - groundPoints[i][1];
        if (i == 0 || (difference < 0 && difference < maxDifference)) {
            maxDifference = difference;
            largestIndex = i;
        }
    }

    // If maxDifference is still 1, all the wheels are above the ground
    if (maxDifference < 0) {
        // The wheel is below the ground surface, we need to move the car up by the
        // difference
        //this->_position[1] += -1 * maxDifference;
        float tmp[3];
        this->_wheels[largestIndex]->getGroundContact(tmp);

        // Get the vector from cog to contact point
        Vector cog = Vector(this->_center, 3);
        Vector ground = Vector(tmp, 3);
        Vector r = cog - ground;

        // Add the linear impulse
        float j = this->_calculateImpulseOnGround(r);

        // v' = v + Jn / m
        // n is pointing up becaus COG of earth is so far away in that direction
        Vector n(0, 1, 0);
        Vector linearVelocity(this->_linearVelocity, 3);

        Vector dLinearVelocity = (j * n) / this->_mass;
        linearVelocity = linearVelocity + dLinearVelocity;
        this->_linearVelocity[0] = linearVelocity[0];
        this->_linearVelocity[1] = linearVelocity[1];
        this->_linearVelocity[2] = linearVelocity[2];

        // Now we attend to the angular velocity
        Vector angularVelocity(this->_angularVelocity, 3);
        angularVelocity += ( r ^ ((j * n) / this->_mass) ) * this->_inverseInertiaTensor;
        this->_angularVelocity[0] = angularVelocity[0];
        this->_angularVelocity[1] = angularVelocity[1];
        this->_angularVelocity[2] = angularVelocity[2];
    }
}

float Car::_calculateImpulseOnGround(Vector & r) {
    // J = impulse
    // v = velocity of the car
    // e = coefficient of restitution
    // r = vector from car COG to contact point
    // m1 = mass of car
    // m2 = mass of earth - 1 / m2 tends to 0, so we don't need it
    // n = unit vector from earch COG to car COG

    // We choose a very elastic coefficient
    float e = 0.5;
    float tmp;
    Vector v(this->_linearVelocity, 3);
    Vector n(0, 1, 0);

    // With moments
    // J = -(v . n) * (e + 1) 
    //     / 
    //     (1/m1 + 1/m2 + n . [(r1 x n) / I1] x r1 + n . [(r2 x n) / I2] x r2)
    Vector zero(0, 0, 0);
    tmp = n * ( ( (r ^ n) * this->_inverseInertiaTensor ) ^ r ) + n * zero;
    tmp = (v * n) * (-1 * (e + 1) ) / (1.0 / this->_mass + tmp);

    return tmp;
}

/******************************************************************************
 * Physics stuff
 *****************************************************************************/
void Car::_calculateMovement() {
    Vector accumulativeForce(0, 0, 0);
    Vector accumulativeMoment(0, 0, 0);
    Vector gravityForce(3);
    Vector yAxis(0, -9.8, 0);
    float time = this->_timer->getSeconds();
    Vector acceleration(3);
    Vector dragForce(3);
    Vector linearVelocity(this->_linearVelocity, 3);
    Vector angularVelocity(this->_angularVelocity, 3);

    // Calculate the force of gravity
    gravityForce = this->_mass * yAxis;
    accumulativeForce += gravityForce;

    this->_calculateWheelForces(accumulativeForce, accumulativeMoment);

    // Add drag
    // NOTE _mass ^ 2 here needs checking, but will do for now...
    float drag = (-1.0 / 391.0) * this->_mass * this->_mass 
        * this->_bodyArea * this->_dragCoefficient;
    dragForce = drag * linearVelocity;
    accumulativeForce += dragForce;


    // convert sum of forces into acceleration vector
    acceleration = (1.0 / this->_mass) * accumulativeForce;

    // Scale the force by time
    acceleration = acceleration * time;

    // Apply the force to the vector
    //vertexAdd(acceleration, this->_linearVelocity, this->_linearVelocity);
    this->_linearVelocity[0] += acceleration[0];
    this->_linearVelocity[1] += acceleration[1];
    this->_linearVelocity[2] += acceleration[2];

    // Now we apply the moments to the angular velocity
    Vector tmp = angularVelocity ^ (this->_inertiaTensor * angularVelocity);
    tmp = this->_inverseInertiaTensor * (time * (accumulativeMoment - tmp));
    this->_angularVelocity[0] += tmp[0];
    this->_angularVelocity[1] += tmp[1];
    this->_angularVelocity[2] += tmp[2];

    // dampen the angular velocity - this is to correct for rotation friction of the
    // tyres, which i've not modelled properly yet
    vertexMultiply(0.9, this->_angularVelocity, this->_angularVelocity);

    // Now apply angular velocity to the orientation
    Quaternion tmpQ;
    this->_orientation.multiply(this->_angularVelocity, tmpQ);
    tmpQ.multiply(0.5 * time);
    this->_orientation.add(tmpQ);

    // Now we need to normalise the quaternion
    this->_orientation.normalise();
}

void Car::_calculateWheelForces(Vector & forceAccumulator, Vector & angularAccumulator) {
    float wheelPoint[3];
    float groundPoint[3];
    Vector tmpForce(3);
    bool wheelOnGround[4];
    bool wheelBelowGround[4];
    int wheelOnGroundCount = 0;
    Vector gravityForce(3);
    Vector yAxis(0, 1, 0);
    float gravityAcceleration = -9.8;
    Vector forward(0, 0, 1);
    Vector center(this->_center, 3);
    Vector linearVelocity(this->_linearVelocity, 3);
    Vector vGroundPoint(3);
    Vector vPoint(3);
    Vector r(3);

    float * point;
    Matrix orientation;
    Matrix wheelMatrix;
    Matrix opposite;
    float distanceFromCOG[4];
    float totalDistanceFromCOG = 0;


    // Find out which wheels are on the ground
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->getGroundContact(wheelPoint);
        this->_matrix.multiplyVector(wheelPoint);

        groundPoint[0] = this->_closestGroundPoints[i][0];
        groundPoint[1] = this->_closestGroundPoints[i][1];
        groundPoint[2] = this->_closestGroundPoints[i][2];

        // If the ground point is >= wheel point, the car is on the ground
        // NOTE: we're giving the wheel a 5cm leaway there
        if (groundPoint[1] + 0.05 >= wheelPoint[1]) {
            wheelOnGround[i] = true;
            // We need a count of how many wheels are on the ground so that we know 
            // how much weight is on each one
            ++wheelOnGroundCount;
        } else {
            wheelOnGround[i] = false;
        }

        // We also want to know if the wheel is below the ground
        wheelBelowGround[i] = groundPoint[1] > wheelPoint[1];
    }

    // Calculate the force of gravity
    gravityForce = this->_mass * gravityAcceleration * yAxis;

    // We will need a rotaton matrix for the orientation of the car
    this->_orientation.toRotationMatrix(orientation);
    orientation.invert(opposite);

    // Calculate the distance from the center of gravity for each wheel, the amount
    // of weight carried by a wheel is inversely proportional to its distance from the 
    // COG
    for (int i = 0; i < 4; ++i) {
        if (wheelOnGround[i]) {
            this->_wheels[i]->getGroundContact(groundPoint);
            vGroundPoint = Vector(groundPoint, 3);

            r = momentDistance(vGroundPoint, yAxis, center);

            distanceFromCOG[i] = 1.0 / r.magnitude();
            totalDistanceFromCOG += distanceFromCOG[i];
        }
    }

    if (totalDistanceFromCOG > 0) {
        forceAccumulator += -gravityForce;
    }

    // Add the forces for each wheel
    for (int i = 0; i < 4; ++i) {
        point = this->_wheels[i]->getWheelCenter();
        vPoint = Vector(point, 3);
        this->_wheels[i]->getGroundContact(groundPoint);
        vGroundPoint = Vector(groundPoint, 3);

        // If the wheel is on the ground we add a upforce
        if (wheelOnGround[i]) {
            float weightRatio = distanceFromCOG[i] / totalDistanceFromCOG;

            tmpForce = -weightRatio * gravityForce;

            // And effect the angular momentum
            tmpForce = opposite * tmpForce;

            r = momentDistance(vGroundPoint, tmpForce, center);
            tmpForce = r ^ tmpForce;
            angularAccumulator += tmpForce;
        } 

        // Check if this is a powered wheel
        if (this->_wheels[i]->isPowered && wheelOnGround[i]) {
            // Get the forward direction of the car in world coordinates
            //orientation.multiplyVector(forward, tmpForce);
            // TODO, this will break becaues of mismatched orders
            tmpForce = orientation * forward;

            // Add a force in the local X direction related to the torque
            float torque = this->_engine.calculateTorque() / (2.0 * 0.3045);
            tmpForce = torque * tmpForce;

            forceAccumulator += tmpForce;

            // Add this force to the rotational forces
            tmpForce = opposite * tmpForce;
            r = momentDistance(vPoint, tmpForce, center);
            //tmpForce = r ^ tmpForce;

            // TODO: this is very unstable at the moment
            //angularAccumulator[0] += tmpForce[0];
            //angularAccumulator[1] += tmpForce[1];
            //angularAccumulator[2] += tmpForce[2];
        }

        // Find the wheel vector using the wheel angle and the car's orientation
        // TODO: this is very simple, needs improving + back tyres have friction too
        if (wheelOnGround[i] && this->_wheels[i]->isSteering()) {
            wheelMatrix.reset();
            wheelMatrix.rotateY(-this->_wheels[i]->getAngle());
            wheelMatrix.multiplyMatrix(&orientation);

            tmpForce = wheelMatrix * forward;

            // The forward is actually backwards and is the same length as _linearVel
            tmpForce = -linearVelocity.magnitude() * tmpForce;
            //tmpForce = this->_mass * tmpForce;

            // Find the L vector 
            Vector lVector = linearVelocity + tmpForce;
            lVector = (lVector ) * this->_mass ;

            // Put the L vector back into car's local system
            //lVector.print();

            //forceAccumulator += lVector;


            lVector = opposite * lVector;
            this->_wheelVectors[i] = (lVector / lVector.magnitude()) + vGroundPoint;

            //tmpForce = tmpForce * (linearVelocity.magnitude() * this->_mass);
            //this->_wheelVectors[i] = (tmpForce / tmpForce.magnitude()) + vGroundPoint;

            r = momentDistance(vGroundPoint, lVector, center);
            tmpForce = vGroundPoint ^ lVector;

            //this->_wheelVectors[i] = (tmpForce / tmpForce.magnitude()) + vGroundPoint;

            angularAccumulator += tmpForce;
        }
    }
}

void Car::_calculateInertiaTensor() {
    // At the moment we're not adding any extra elements to the car (wheels, gas tank, 
    // etc) so the inertia tensor is pretty simple.
    this->_inertiaTensor[0] = this->_inertia[0];
    this->_inertiaTensor[1] = 0;
    this->_inertiaTensor[2] = 0;

    this->_inertiaTensor[3] = 0;
    this->_inertiaTensor[4] = this->_inertia[1];
    this->_inertiaTensor[5] = 0;

    this->_inertiaTensor[6] = 0;
    this->_inertiaTensor[7] = 0;
    this->_inertiaTensor[8] = this->_inertia[2];

    this->_inertiaTensor.invert(this->_inverseInertiaTensor);
    this->_inverseInertiaTensor.print();
}


