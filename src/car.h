/**
 * Class representing a car. This includes (so far) its rendering, animation and movement.
 *
 * TODO:
 *  * Use a turning circle to calculate the car's rotation
 *  * Clean up memory, this will be important when there are opponents
 */
#pragma once

#include <SDL/SDL.h>
#include <ode/ode.h>


#include "wheel.h"
#include "track.h"
#include "dof.h"
#include "quaternion.h"
#include "vector.h"
#include "drive_systems.h"
#include "rigid_body.h"
#include "frame_timer.h"

class Dof;
class Engine;
class Gearbox;

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
        void setEngine(Engine & engine);
        void setGearbox(Gearbox & gearbox);
        void setDimensions(float height, float width, float length);
        void setCenter(float * center);
        void setMass(float mass, float * inertia);

        // Getters
        Engine * getEngine();

        float * getVector();

        // Update the car's position and orientation at 1000Hz
        static void * update(void * car);

        // The rigid body id
        dBodyID bodyId;
        // and collision box id
        dGeomID geomId;

        FrameTimer * timer;

    private:
        Wheel * _wheels[4];

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
        Engine * _engine;
        float _engineRPM;
        float _engineMaxRPM;
        bool _acceleratorPressed;
        float _acceleratorRPMSec;

        // Gear related variables
        Gearbox * _gearbox;
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

        void _initRigidBody();

        // Update the collision detection box
        void _updateCollisionBox();

        // The collision joints
        int _nJoints;
        dJointID _joints[4];
};
