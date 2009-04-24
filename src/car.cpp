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

    this->_localOrigin[0][0] = -1;
    this->_localOrigin[0][1] = 0;
    this->_localOrigin[0][2] = 0;

    this->_localOrigin[1][0] = 0;
    this->_localOrigin[1][1] = 1;
    this->_localOrigin[1][2] = 0;

    this->_localOrigin[2][0] = 0;
    this->_localOrigin[2][1] = 0;
    this->_localOrigin[2][2] = -1;

    /*
    this->_angularVelocity[0] = 0;
    this->_angularVelocity[1] = 0;
    this->_angularVelocity[2] = 0;
    */


    this->initialiseRigidBody();
}

Car::~Car() {
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
    /*
    // If the accelerator is pressed we increase the RPM be the accerelation coefficient
    if (this->_acceleratorPressed) {
        //this->_engineRPM += this->_acceleratorRPMSec * this->_timer->getSeconds();
        this->_engine->getCurrentRpm() += this->_acceleratorRPMSec * this->_timer->getSeconds();
    // ... otherwise we decrease the RPM
    } else if (this->_engine->getCurrentRpm() > 0) {
        //this->_engineRPM -= (this->_acceleratorRPMSec / 2.0) 
            // this->_timer->getSeconds();
        this->_engine->getCurrentRpm() -= (this->_acceleratorRPMSec / 2.0) 
            * this->_timer->getSeconds();
        // if this is negative, clamp to 0
        if (this->_engine->getCurrentRpm() < 0) this->_engine->getCurrentRpm() = 0;
    }

    // Update the gears
    this->_gearbox->doShift();
    */

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
    // Update the car's steering
    this->_updateSteering();

    // Update the engine
    this->_updateEngine();

    // Find out how much the engine has turned
    float engineTurns = this->_engine->getCurrentRpm() * this->timer->getSeconds() * 60.0;
    // And divide by current gear ratio
    float wheelTurns = engineTurns / this->_gearRatios[this->_currentGear];
    // ... and divide by the final drive axis ratio
    wheelTurns = wheelTurns / this->_finalDriveAxisRatio;
    // ... and turn the wheels
    for (int i = 0; i < 4; ++i) {
        this->_wheels[i]->turn(wheelTurns * 360.0);
    }

    // Add the gravity force
    this->addForce(Vector(0, -9.8 * this->mass, 0));

    /*
    float * tmp;
    tmp = this->_wheels[2]->getWheelCenter();
    Vector v = Vector(tmp, 3);
    this->_wheelVectors[2] = v;
    this->addLocalForce(Vector(0, 2, 0), v);

    tmp = this->_wheels[0]->getWheelCenter();
    v = Vector(tmp, 3);
    this->_wheelVectors[0] = v;
    this->addLocalForce(Vector(0, 2, 0), v);
    */

    // Add the ground contact forces
    //this->_generateGroundForces();

    // Update the car's vector
    this->calculate();

    // Check for collision with the ground on this new position
    this->_groundCollisionCorrection();
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

        car->timer->newFrame();
        car->_updateComponents();
        d = car->timer->getTimeTillNext();
        SDL_Delay(d);
    }
}

void Car::render() {
    glPushMatrix();

    this->_mutex.lock();
    glMultMatrixf(this->transformationMatrix.getMatrix());
    this->_mutex.unlock();

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
Vector Car::getPosition() {
    return this->position;
}

float Car::getRPM() {
    return this->_engine->getCurrentRpm();
}

float * Car::getWheelPosition() {
    float * result = new float[3];
    result[0] = 0.953;
    result[1] = 1.434;
    result[2] = 0.342;
    // transform the vertex
    this->transformationMatrix.multiplyVector(result);
    return result;
}

void Car::setBody(Dof * dof) {
    this->_bodyDof = dof;
}

void Car::setTrack(Track * track) {
    this->_track = track;

    // Set the start position to the position on the track
    this->position = Vector(this->_track->startPosition, 3);

    // Add to the position
    this->position[1] += 4;
}

void Car::setWheel(Wheel * wheel, int index) {
    this->_wheels[index] = wheel;
}


float * Car::getVector() {
    return this->_localOrigin[0];
}


void Car::setBodyArea(float area) {
    this->_bodyArea = area;
}

void Car::setDragCoefficient(float coefficient) {
    this->_dragCoefficient = coefficient;
}

void Car::setEngine(Engine & engine) {
    this->_engine = new Engine();
    *(this->_engine) = engine;
    this->_engine->print();
}

void Car::setGearbox(Gearbox & gearbox) {
    this->_gearbox = new Gearbox();
    *(this->_gearbox) = gearbox;
    this->_gearbox->setCar(this);
}

Engine * Car::getEngine() {
    return this->_engine;
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
        this->transformationMatrix.multiplyVector(wheelPoint);

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
        this->transformationMatrix.multiplyVector(wheelPoints[i]);
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

    // If we have a wheel below the surface, we move the car up
    if (maxDifference < 0) {
        this->position[1] -= maxDifference;
        this->_updateMatrices();
    }

    Vector contactPoints[4];
    Vector impulses[4];

    Matrix inverseRotation(4);
    this->rotationMatrix.invert(inverseRotation);
    Matrix worldInertiaTensor = 
        (this->rotationMatrix * this->inverseInertiaTensor) * inverseRotation;

    // Check for collisions on the contact wheel
    int count = 0;
    //for (int i = 0; i < 4; ++i) {
    for (int i = largestIndex; i < largestIndex + 1; ++i) {
        // We need to redo the wheel points, in case there is a difference
        this->_wheels[i]->getGroundContact(wheelPoints[i]);
        this->transformationMatrix.multiplyVector(wheelPoints[i]);
        difference = wheelPoints[i][1] - groundPoints[i][1];
        if (difference > 0.05) {
            this->_wheelVectors[i] = Vector(0, 0, 0);
            impulses[i] = Vector(0, 0, 0);
            contactPoints[i] = Vector(0, 0, 0);
            continue;
        }

        float tmp[3];
        this->_wheels[i]->getGroundContact(tmp);
        Vector groundContact = Vector(tmp, 3);
        Vector contactNormal = Vector(0, 1, 0);

        Vector contact = this->rotationMatrix * groundContact;

        Matrix impulseToTorque = contact.toSkewSymmetric();
        Matrix deltaVelWorld = 
            ((impulseToTorque * worldInertiaTensor) * impulseToTorque) * -1;

        // We skip the conversion to world coordinates because they are othogonal to the
        // contact coordinates
        Matrix deltaVelocity = deltaVelWorld;

        // Add the linear element
        deltaVelocity[0] += 1.0 / this->mass;
        deltaVelocity[4] += 1.0 / this->mass;
        deltaVelocity[8] += 1.0 / this->mass;

        // Invert to get the impulse needed per unit velocity
        Matrix impulseMatrix;
        deltaVelocity.invert(impulseMatrix);

        // Get the car's velocity
        Vector velocity = this->angularVelocity ^ contact;
        velocity += this->linearVelocity;

        // Coefficient of restitution
        float restitution = 0.1; // Stable at 0.025
        float velocityLimit = 2.5; // Millington 0.25
        float velocityFromAcceleration = this->accelerationAtUpdate * contactNormal;
        if (fabs(velocity[1]) < velocityLimit) {
            restitution = 0;
        }
        float targetDeltaVelocity = 
            -velocity[1] - restitution * (velocity[1] + velocityFromAcceleration);

        // Find the target velocities to kill
        cout << "Target delta velocity: ";
        cout << targetDeltaVelocity << endl;
        Vector velKill(-velocity[0], targetDeltaVelocity, -velocity[2]);
        velKill *= 0.001;

        Vector impulseContact = impulseMatrix * velKill;

        cout << "Impulse: " << endl;
        impulseContact.print();
        impulses[i] = impulseContact;
        contactPoints[i] = contact;

        // Get the relative contact position in world coordinates
        /*
        
        Vector torquePerUnitImpuse = contact ^ contactNormal;
        Vector rotationPerUnitImpulse = worldInertiaTensor * torquePerUnitImpuse;
        Vector velocityPerUnitImpulse = rotationPerUnitImpulse ^ contact;

        // The angular component is the velocity per unit impulse in the up direction
        float deltaVelocityPerImpulse = velocityPerUnitImpulse[1];
        // ... and add the linear component
        deltaVelocityPerImpulse += 1.0 / this->mass;
        //cout << "Target delta velocity: " << targetDeltaVelocity << endl;

        // Get the car's velocity
        Vector velocity = this->angularVelocity ^ contact;
        velocity += this->linearVelocity;

        // Coefficient of restitution
        float restitution = 0.1; // Stable at 0.025
        float velocityLimit = 2.5; // Millington 0.25
        float velocityFromAcceleration = this->accelerationAtUpdate * contactNormal;
        if (fabs(velocity[1]) < velocityLimit) {
            cout << "Resitution reset" << endl;
            restitution = 0;
        }
        //float targetDeltaVelocity = -velocity[1] * (1 + restitution);
        float targetDeltaVelocity = 
            -velocity[1] - restitution * (velocity[1] + velocityFromAcceleration);
        float tmpDV = targetDeltaVelocity / deltaVelocityPerImpulse;
        Vector impulse = Vector(0, tmpDV, 0);
        this->_wheelVectors[i] = (impulse / impulse.magnitude()) + groundContact;
        impulses[i] = impulse;
        contactPoints[i] = contact;
        */
        count++;
        this->applyImpulse(impulses[i], contactPoints[i]);
        this->_updateMatrices();
    }
    cout << "Count: " << count << endl;

    // We've delayed adding the impulses to now, so that all the impulses are calculated
    // from the same linear and angular velocities
    for (int i = 0; i < 4; ++i) {
    }

}

/******************************************************************************
 * Physics stuff
 *****************************************************************************/

