/**
 * Class representing a wheel.
 */
#pragma once

#include <ode/ode.h>
#include <vector>

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
        void setRollingCoefficient(float coefficient);
        void setMaxBrakeTorque(float torque);

        // Set the lateral Pacejka constants
        void setLateralPacejka(float a0, float a1, float a2, float a3, float a4, float a5,
                float a6, float a7, float a8, float a9, float a10, float a11, float a12,
                float a13, float a14);

        void setLongPacejka(float b0, float b1, float b2, float b3, float b4, float b5,
                float b6, float b7, float b8, float b9, float b10, float b11, float b12);

        // Calculate the lateral force using Pacejka's formula and tyre constants
        float calculateLateralPacejka();

        // Calculate the logitudinal force
        float calculateLongPacejka();

        // Get the rolling resitance of this wheel
        float calculateRollingResitance();


        bool isPowered;

        dBodyID bodyId;
        dGeomID geomId;
        dJointID suspensionJointId;

        // Update the angular velocity of the wheel based on the various torques acting
        // on the wheel such as engine, brakes and rolling resitance
        void updateRotation();

        // Apply an amount of braking to this wheel. This should be a value between 0 
        // and 1
        void applyBrake(float amount);
        float braking;

        // Calculate the torque generated on the wheel by the brakes
        float calculateBrakeTorque();

    private:
        // The dof model representing the wheel
        Dof * _dof;
        // ... and representing the brake
        Dof * _brakeDof;

        // Pointer to the car this wheel is attached to
        Car * car;

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

        // The rolling coefficient of the tyre. The rolling resitance is this multiplied
        // by the weight on the tyre
        float _rollingCoefficient;

        // The lateral pacejka constants for this tyre
        std::vector<float> _lateralPacejka;
        std::vector<float> _longPacejka;

        // The angular velocity and rotation of the wheel. We use our own here because
        // we're only interesting in the angular velocity in the wheel's plane of 
        // symmetry.
        float angularVelocity;
        float rotation;

        // We need the inertia for the drive system calculations
        float inertia;

        // The radius is needed for wheel rotation / angular velocity calculations
        float radius;

        // The maximum torque produced by the brakes
        float maxBrakeTorque;
};
