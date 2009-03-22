#include "quaternion.h"

#include <math.h>

Quaternion::Quaternion() {
    this->w = 0;
    this->x = 0;
    this->y = 0;
    this->z = 0;
}

Quaternion::Quaternion(float w, float x, float y, float z) {
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
}

void Quaternion::toRotationMatrix(Matrix & result) {
    result[0] = 1 - 2 * (this->y * this->y + this->z * this->z);
    result[1] = 2 * (this->x * this->y - this->z * this->w);
    result[2] = 2 * (this->x * this->z + this->y * this->w);
    result[3] = 0;
    result[4] = 2 * (this->x * this->y + this->z * this->w);
    result[5] = 1 - 2 * (this->x * this->x + this->z * this->z);
    result[6] = 2 * (this->y * this->z - this->x * this->w);
    result[7] = 0;
    result[8] = 2 * (this->x * this->z - this->y * this->w);
    result[9] = 2 * (this->y * this->z + this->x * this->w);
    result[10] = 1 - 2 * (this->x * this->x + this->y * this->y);
    result[11] = 0;
    result[12] = 0;
    result[13] = 0;
    result[14] = 0;
    result[15] = 1;
}

void Quaternion::fromEuler(float * euler) {
    // NOTE euler -> { yaw, pitch, roll }
    float c1 = cos(euler[0] / 2);
    float c2 = cos(euler[1] / 2);
    float c3 = cos(euler[2] / 2);
    float s1 = sin(euler[0] / 2);
    float s2 = sin(euler[1] / 2);
    float s3 = sin(euler[2] / 2);

    this->w = c1 * c2 * c3 - s1 * s2 * s3;
    this->x = s1 * s2 * c3 + c1 * c2 * s3;
    this->y = s1 * c2 * c3 + c1 * s2 * s3;
    this->z = c1 * s2 * c3 - s1 * c2 * s3;
}

void Quaternion::toEuler(float * euler) {
    // TODO: I'm missing a special case here for the poles. Though this function 
    // shouldn't really be used except for debugging.
    euler[0] = atan2(
            (2 * (this->w * this->y + this->x * this->z)), 
            (1 - 2 * (this->z * this->z + this->y * this->y)));
    euler[1] = asin(2 * (this->x * this->y + this->z * this->w));
    euler[2] = atan2((2 * (this->w * this->x + this->y * this->z)), 
            (1 - 2 * (this->x * this->x + this->z * this->z)));
}
