/**
 * Class representing a rigid body. It is meant to be sub-classed by an in-game object
 */
#pragma once

#include "matrix.h"
#include "quaternion.h"
#include "vector.h"
#include "frame_timer.h"

#include <math.h>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

// Dampen linear velocity as % per second
#define LINEAR_DAMPER 0.995
// ... and for angular velocity
#define ANGULAR_DAMPER 0.995

using namespace std;
using namespace boost::interprocess;

class RigidBody {
    public:
        ~RigidBody();
        void initialiseRigidBody();

        // Calculate the new position and orientation
        void calculate();

        // Reset all the accumulators ready for a new lot of forces
        void resetAccumulators();

        // Add a force (though the COG)
        void addForce(const Vector & force);

        // Add a force (through the point) in global coordinates
        void addForce(const Vector & force, const Vector & point);

        // Add a force (through a point) in local coordinates
        void addLocalForce(const Vector & force, const Vector & point);

        // Add a force (global coordinates) with a local coordinate point
        void addForceLocalPoint(const Vector & force, const Vector & point);

        // Apply an impulse at position, both in world coordinates
        void applyImpulse(const Vector & impulse, const Vector & point);

        // Setters
        void setCenter(float * center);
        void setInertia(float * inertia);
        void setMass(float mass);

        // The position of the body
        Vector position;

        // The linear velocity of the body
        Vector linearVelocity;
        // ... and force accumulator
        Vector forceAccumulator;

        // The previous frame's acceleration, for the impulse generator
        Vector accelerationAtUpdate;

        // The angular velocity of the body
        Vector angularVelocity;
        // ... and the torque accumulator
        Vector torqueAccumulator;

        // Inertia and inverse tensors
        Matrix inertiaTensor;
        Matrix inverseInertiaTensor;

        // The center of gravity of the body
        Vector cog;

        // The mass of the body
        float mass;

        // Quaternion representing the orientation of the body
        Quaternion orientation;

        // The body's transformation matrix, calculated from the position and orientation
        Matrix transformationMatrix;

        // The rotational component of the transformatino matrix
        Matrix rotationMatrix;

        // A timer to help with the physics calculations
        FrameTimer * timer;

    protected:
        // Mutex to make sure physics and rendering don't confuse each other
        interprocess_mutex _mutex;

        void _updateMatrices();
};
