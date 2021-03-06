/**
 * Class representing a car. This includes (so far) its rendering, animation and movement.
 *
 * TODO:
 *  * Clean up memory, this will be important when there are opponents
 */
#pragma once

#include <SDL/SDL.h>
#include <ode/ode.h>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

#include "wheel.h"
#include "track.h"
#include "dof.h"
#include "quaternion.h"
#include "vector.h"
#include "drive_systems.h"
#include "rigid_body.h"
#include "frame_timer.h"

class Dof;
class Wheel;

class Car {
    public:
        Car();
        ~Car();
        void render();

        // Handle key presses
        void handleKeyPress(SDL_Event &event);

        // Get a pointer to the player's position
        Vector getPosition();

        float getRPM();

        float * getWheelPosition();

        // Setters
        void setBody(Dof * dof);
        void setTrack(Track * track);
        void setWheel(Wheel * wheel, int index);
        void setBodyArea(float area);
        void setDragCoefficient(float coefficient);;
        void setDimensions(float height, float width, float length);
        void setCenter(float * center);
        void setMass(float mass, float * inertia);
        void setBrakeModel(Dof * brake);

        // Getters
        Engine & getEngine();
        Gearbox & getGearbox();
        int getCurrentGear();

        // Calculate the current speed from the car body's velocity vector. Speed should
        // be in m/s
        float getSpeed();

        void getVector(vector<float> & result);

        // Update the car's position and orientation at 1000Hz
        static void * update(void * car);

        // The rigid body id
        dBodyID bodyId;
        // and collision box id
        dGeomID geomId;
        dSpaceID spaceId;

        FrameTimer * timer;
        interprocess_mutex mutex;

        // Get the average slip from the drive wheels
        float maxSlip();

    private:
        boost::ptr_vector<Wheel> wheels;

        Vector _wheelVectors[4];
        Vector _closestGroundPoints[4];
        void _updateGroundPoints();

        // Needs a reference to the track to do collision detection
        Track * _track;

        // The sideways angle of the wheels for steering animation, this is +/- angle away
        // from front facing
        float _wheelsAngle;

        // The angle away from rest of the steering wheel, - is left, + is right
        float _steeringAngle;
        // How much a keypress changes the steeringAngle
        float _steeringDelta;
        float _currentSteering;
        float _maxSteeringAngle;

        // Model scale -> puts the model into meters's
        float _modelScale;

        // Engine related variables
        Engine _engine;
        bool _acceleratorPressed;

        // Gear related variables
        Gearbox gearbox;
        float _finalDriveAxisRatio;
        float * _gearRatios;
        int _numberOfGears;
        int _currentGear;
        float _wheelDiameter;
        float _engineGearUpRPM;
        float _engineGearDownRPM;

        // Drag coefficient and body area
        float _dragCoefficient;
        float _bodyArea;


        // The matrix representing the local coordinate system of the car
        float _localOrigin[3][3];
        // The quaternion representing the yaw, pitch and roll of the car
        //Quaternion _orientation;

        // Update the engine variables
        void _updateEngine();
        // Update the moving components of the car such as engine and wheels
        void _updateComponents();
        // Update the car's steering
        void _updateSteering();

        // Return true of one of the wheels is on the ground
        bool _isOnGround();

        // Check if there was a collision with the ground and correct for it
        void _groundCollisionCorrection();

        // Calculate the forces and their effect on the linear and angular velocities
        void _calculateMovement();

        // Calculate the forces due to the wheels (contact with the ground and turning)
        void _calculateWheelForces(Vector & forceAccumulator, Vector & angularAccumulator);
        // Generate the forces of the ground pushing up
        void _generateGroundForces();

        Dof * _bodyDof;

        // The model for the brake lights
        Dof * brakeModel;
        bool brakePressed;

        void _initRigidBody();

        // Update the collision detection box
        void _updateCollisionBox();

        // Add the forces for this step
        void _addForces();


        // The collision joints
        int _nJoints;
        dJointID _joints[4];

        // Press and release brakes
        void pressBrake();
        void releaseBrake();
};
