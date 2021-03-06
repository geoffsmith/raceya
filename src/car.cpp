#include "car.h"
#include "matrix.h"
#include "lib.h"
#include "frame_timer.h"
#include "closest_point.h"
#include "logger.h"

#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <math.h>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/if.hpp>
#include <boost/foreach.hpp>

#define PI 3.14159265

using namespace std;
using namespace boost::lambda;

Car::Car() : gearbox(*this), _engine(*this) {
    this->_modelScale = 1;

    this->_wheelDiameter = 0.645; // meters;
    this->_wheelsAngle = 0;
    this->_steeringAngle = 0;
    this->_steeringDelta = 5; // degrees
    this->_currentSteering = 0;
    this->_maxSteeringAngle = 60; // degrees

    // Set up the engine
    this->_acceleratorPressed = false;

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

    this->timer = new FrameTimer(200);

    this->_initRigidBody();

    this->brakeModel = NULL;
    this->brakePressed = false;
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


    // rotate the front wheel according to the steering
    this->_wheelsAngle = this->_steeringAngle;

    // Update the rotation of the front wheels
    BOOST_FOREACH (Wheel & wheel, this->wheels) {
        if (wheel.isSteering()) {
            wheel.setAngle(this->_wheelsAngle);
        }
    }
}

void Car::_updateEngine() {
    // Accelerate if accelerator pressed, decelerate otherwise
    /*
    if (this->_acceleratorPressed) {
        this->_engine->accelerate(this->timer->getSeconds());
    } else {
        this->_engine->decelerate(this->timer->getSeconds());
    }
    */

    // Since we have a new RPM, do a shift
    this->gearbox.doShift();
}

void Car::_updateComponents() {
    // Update the car's steering
    this->_updateSteering();

    // And the engine
    this->_updateEngine();
}

void * Car::update(void * _car) {
    Car * car = (Car *)_car;
    float d;
    while (true) {
        car->timer->newFrame();
        car->_updateComponents();
        // TODO: Sort out this constant step business
        //car->_addForces();
        //car->_updateCollisionBox();

        car->mutex.lock();
        //dWorldStep(Track::worldId, car->timer->getTargetSeconds());
        car->mutex.unlock();

        // Delete the joints
        for (int i = 0; i < car->_nJoints; ++i) {
            //dJointDestroy(car->_joints[i]);
        }

        d = car->timer->getTimeTillNext();
    }
}

void Car::render() {
    glPushMatrix();
    this->mutex.lock();

    const dReal * position = dBodyGetPosition(this->bodyId);
    glTranslatef(position[0], position[1], position[2]);

    // Get the car's rotation
    const dReal * rotation = dBodyGetRotation(this->bodyId);
    Matrix rotationMatrix(rotation, 3);
    glMultMatrixf(rotationMatrix.getMatrix());

    this->_bodyDof->render(true);

    // Render the brake model if we have one and brake is pressed
    if (this->brakePressed && this->brakeModel != NULL) {
        this->brakeModel->render(true);
    }

    glPopMatrix();

    // Render the wheels
    std::for_each(
            this->wheels.begin(), 
            this->wheels.end(), 
            std::mem_fun_ref(&Wheel::render));

    this->mutex.unlock();

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
                    this->_engine.pressAccelerator();
                    break;
                case SDLK_DOWN:
                    this->pressBrake();
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
                case SDLK_UP:
                    this->_acceleratorPressed = false;
                    this->_engine.releaseAccelerator();
                    break;
                case SDLK_DOWN:
                    this->releaseBrake();
                    break;
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
float Car::getSpeed() {
    const dReal * bodyVelocity = dBodyGetLinearVel(this->bodyId);

    // convert to the car's space so we can take out the Z component
    dVector3 result;
    dBodyVectorFromWorld(
            this->bodyId, 
            bodyVelocity[0], bodyVelocity[1], bodyVelocity[2],
            result);
    return result[2];
}

Vector Car::getPosition() {
    const dReal * position = dBodyGetPosition(this->bodyId);
    Vector result(position[0], position[1], position[2]);
    return result;
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
    //this->transformationMatrix.multiplyVector(result);
    return result;
}

void Car::setBody(Dof * dof) {
    this->_bodyDof = dof;
}

void Car::setBrakeModel(Dof * brake) {
    this->brakeModel = brake;
}

void Car::setTrack(Track * track) {
    this->_track = track;
    dBodySetPosition(this->bodyId, 
            this->_track->startPosition[0], 
            this->_track->startPosition[1] + 1, 
            this->_track->startPosition[2]);

    // Set the wheel positions
    const dReal * position = dBodyGetPosition(this->bodyId);
    std::for_each(
            this->wheels.begin(), 
            this->wheels.end(), 
            bind(&Wheel::setCarPosition, _1, position));
}

void Car::setWheel(Wheel * wheel, int index) {
    this->wheels.push_back(wheel);
}


void Car::getVector(vector<float> & result) {
    result = vector<float>(3);
    dVector3 tmp;
    dBodyVectorToWorld(this->bodyId, 0, 0, 1, tmp);
    result[0] = tmp[0];
    result[1] = tmp[1];
    result[2] = tmp[2];
}


void Car::setBodyArea(float area) {
    this->_bodyArea = area;
}

void Car::setDragCoefficient(float coefficient) {
    this->_dragCoefficient = coefficient;
}

void Car::setDimensions(float height, float width, float length) {
    dGeomBoxSetLengths(this->geomId, height, width, length);
}

Engine & Car::getEngine() {
    return this->_engine;
}

Gearbox & Car::getGearbox() {
    return this->gearbox;
}

void Car::setCenter(float * center) {
}


void Car::setMass(float mass, float * inertia) {
    dMass newMass;

    dMassSetParameters(&newMass, mass, 0, 0, 0,
            inertia[0], inertia[1], inertia[2],
            0, 0, 0);

    dBodySetMass(this->bodyId, &newMass);
}

int Car::getCurrentGear() {
    return this->gearbox.getCurrentGear();
}

void Car::pressBrake() {
    this->brakePressed = true;
    std::for_each(this->wheels.begin(), this->wheels.end(), 
            bind(&Wheel::applyBrake, _1, 0.5));
}

void Car::releaseBrake() {
    this->brakePressed = false;
    std::for_each(this->wheels.begin(), this->wheels.end(), 
            bind(&Wheel::applyBrake, _1, 0));
}

/******************************************************************************
 * Physics stuff
 *****************************************************************************/
void Car::_initRigidBody() {
    // Set up the rigid body
    this->bodyId = dBodyCreate(Track::worldId);

    this->spaceId = dHashSpaceCreate(0);

    // Set up the collision detection box
    this->geomId = dCreateBox(0, 0, 0, 0);

    // Associate geom with body
    dGeomSetBody(this->geomId, this->bodyId);
}

void Car::_updateCollisionBox() {
    // TODO: can this be done with fewer calls to dCollide? Would it make a difference?

    dContactGeom * contacts = new dContactGeom[4];
    dContactGeom tmpContacts[1];
    int count = 0;

    BOOST_FOREACH (Wheel & wheel, this->wheels) {
        int result = dCollide(wheel.geomId, (dGeomID)Track::spaceId, 1, 
                tmpContacts, sizeof(dContactGeom));
        if (result > 0) {
            contacts[count] = tmpContacts[0];
            ++count;
        }
    }

    this->_nJoints = count;
    if (count  > 0) {
        for (int i = 0; i < count; ++i) {
            // Create the surface parameters
            dSurfaceParameters params;
            params.mode = dContactMu2 & dContactBounce;
            params.mu = 10;
            params.bounce = 0;

            dContact contact;
            contact.geom = contacts[i];
            contact.surface = params;
            dJointID jointId = dJointCreateContact(Track::worldId, 0, &contact);
            dBodyID bodyId = dGeomGetBody(contacts[i].g1);
            dJointAttach(jointId, bodyId, 0);
            this->_joints[i] = jointId;
        }
    }
    delete [] contacts;
}

void Car::_addForces() {

    // Get the variables we need for the forces acting on the body
    const dReal * bodyVelocity = dBodyGetLinearVel(this->bodyId);
    float speed = dLENGTH(bodyVelocity);
    float aeroDragC;
    float aeroDrag;
    Vector direction;

    // stop if speed is infinite (as happens at init, worth investigating TODO)
    if (std::numeric_limits<float>::infinity() == speed) {
        return;
    }

    if (speed > 0) {
        direction = Vector( 
                bodyVelocity[0] / speed, 
                bodyVelocity[1] / speed, 
                bodyVelocity[2] / speed );

        // Add aerodynamic drag
        aeroDragC = 0.5 * 1.29 *  this->_dragCoefficient * this->_bodyArea;
        aeroDrag = -aeroDragC * speed * speed;

        dBodyAddForce(this->bodyId,
                aeroDrag * direction[0],
                aeroDrag * direction[1],
                aeroDrag * direction[2]
                );

    }

    // Apply wheel forces
    BOOST_FOREACH (Wheel & wheel, this->wheels) {
        // Update the rotation of the wheel. If this wheel is powered it will calculate
        // the torque on the wheel due to the engine forces etc.
        wheel.updateRotation();

        // Add the lateral pacejka force
        float lateralPacejka = wheel.calculateLateralPacejka();

        // We need to apply the lateral force 90 degrees to the wheel, the sign of the 
        // lateral force will deal with direction
        dBodyAddRelForce(wheel.bodyId, lateralPacejka, 0, 0);

        float longPacejka = wheel.calculateLongPacejka();
        dBodyAddRelForce(wheel.bodyId, 0, 0, longPacejka);
        
    }
}

float Car::maxSlip() {
    // Get the average slip of the driving wheelsH
    float max = -10000;
    BOOST_FOREACH (Wheel & wheel, this->wheels) {
        if (wheel.isPowered) { 
            float slip = wheel.calculateSlip();
            if (slip > max) max = slip;
        }
    }
    return max;
}
