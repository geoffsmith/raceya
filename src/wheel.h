/**
 * Class representing a wheel.
 */
#pragma once

#include "dof.h"
#include "matrix.h"

class Wheel {
    public:
        Wheel(int position, Dof * dof);
        void render();

        // Turn the wheel around its axis
        void turn(float turn);

        // Set the angle away from from facing (for front wheel usually)
        void setAngle(float angle);

        // Get the point at which the wheel touches the ground
        void getGroundContact(float * point);

        void setCenter(float * center);
        void setBrakeDof(Dof * dof);

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

};
