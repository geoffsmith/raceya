/**
 * Class representing a wheel.
 */
#pragma once

#include <ode/ode.h>

#include "dof.h"
#include "matrix.h"
#include "car.h"

class Car;


class Wheel {
    public:
        Wheel(int position, Dof * dof, Car * car);
        ~Wheel();
        void render();

        // Turn the wheel around its axis
        void turn(float turn);

        // Set the angle away from from facing (for front wheel usually)
        void setAngle(float angle);
        float getAngle();

        // Get the point at which the wheel touches the ground
        void getGroundContact(float * point);

        // Get the center of the wheel
        float * getWheelCenter();

        void setCarPosition(const float * position);
        void setCenter(float * center);
        void setBrakeDof(Dof * dof);
        void enableSteering();
        bool isSteering();

        void setRadius(float radius);
        void setMass(float mass, float inertia);

        bool isPowered;

        dBodyID bodyId;
        dGeomID geomId;
        dJointID suspensionJointId;

    private:
        // The dof model representing the wheel
        Dof * _dof;
        // ... and representing the brake
        Dof * _brakeDof;

        // The current rotation of the wheel
        float _rotation;

        // The position of the wheel front, back, left and right
        int _position;

        // The position of the center of the wheel - for rotating around
        float _wheelCenter[3];

        // Lowest point of the wheel
        float _groundContact[3];

        // The angle of the wheel deviating from front facing
        float _wheelAngle;

        // True if this wheel is steered
        bool _steering;
};
