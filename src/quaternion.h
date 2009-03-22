/*
 * Class representing a quaternion and its methods
 */
#pragma once

#include "matrix.h"

class Quaternion {
    public:
        // The actual quaternion data
        float x;
        float y;
        float z;
        float w;

        // Constructor setting everything to 0
        Quaternion();
        // The constructor, straight up from a quaternion
        Quaternion(float w, float x, float y, float z);
        // Constructor from a euler angle vector { yaw, pitch, roll }
        Quaternion(float * euler);

        // Return a rotation matrix representing this quaternion
        void toRotationMatrix(Matrix & rotation);

        // Set the euler vector to { yaw, pitch, roll }
        void toEuler(float * euler);
        // Set this quaternion based on the euler vector
        void fromEuler(float * euler);
};
