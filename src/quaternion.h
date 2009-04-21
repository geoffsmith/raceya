/*
 * Class representing a quaternion and its methods
 */
#pragma once

#include "matrix.h"
#include "vector.h"

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

        // Normalise this quaternion
        void normalise();

        // Multiply with a vector
        void multiply(float * vector);
        void multiply(float * vector, Quaternion & result);
        void multiply(float scalar);


        Quaternion & operator+=(const Quaternion & rhs);

        // Add to quaternions
        void Quaternion::add(Quaternion & quaternion);

        void print() const;
};

inline Quaternion operator*(const Vector & a, const Quaternion & b);
inline Quaternion operator*(const Quaternion & a, const Quaternion & b);
inline Quaternion operator*(const float a, const Quaternion & b);

inline Quaternion operator*(const Vector & a, const Quaternion & b) {
    // First we make a into a quaternion
    Quaternion result;
    Quaternion aq(0, a[0], a[1], a[2]);

    // Now we multiply them together
    result = aq * b;
    return result;
}

inline Quaternion operator*(const Quaternion & a, const Quaternion & b) {
    Quaternion result;

    // This is correct according to millington's code and wikipedia - millingtons 
    // equation is different.
    result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    result.y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
    result.z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;

    return result;
}

inline Quaternion operator*(const float scalar, const Quaternion & b) {
    Quaternion result = b;
    result.w *= scalar;
    result.x *= scalar;
    result.y *= scalar;
    result.z *= scalar;
    return result;
}
