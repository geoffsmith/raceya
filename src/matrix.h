/**
 * A class to represent a transformation matrix. It has a number of helper functions to help
 * build a transformation. These mostly mirror openGL functions, but give us better access to
 * the resulting matrix (and a nicer interface).
 *
 * Performance wise, putting this functionality in a class should be OK, because there 
 * shouldn't be too many matrices floating around (compared with vertices say). However, 
 * methods performed over large number of vertices need to be very fast i.e. multiplyVector
 */
#pragma once

#include "vector.h"

class Vector;

#include <OpenGL/gl.h>
#include <ode/ode.h>

class Matrix {
    public:
        Matrix();
        Matrix(const Matrix & other);
        Matrix(GLfloat * input);
        Matrix(int order);
        Matrix(const dReal * other, int order);

        ~Matrix();
        void reset();
        float& operator[] (const unsigned int index);
        float operator[] (const unsigned int index) const;
        Matrix & operator=(const Matrix & other);
        Matrix & operator*=(const float scalar);
        void rotate(float angle, float * normal);
        void rotateY(float angle);
        void rotateX(float angle);
        void rotateZ(float angle);
        void translate(float x, float y, float z);
        void scale(float scale);
        void multiplyVector(float *vector, float *result);

        // Multiply matrix and vector, store results in original vector
        void multiplyVector(float *vector);

        void multiplyVectorSkipTranslation(float *vector, float *result);

        void multiplyMatrix(Matrix * matrix);
        void multiplyMatrix(float * matrix);
        
        // Calculate the minor matrix of order - 1
        void minor(Matrix & minor, int row, int column);

        // Calculate the determinant of this matrix
        float determinant();

        // Calculate the inverse of this matrix
        void invert(Matrix & matrix);
        Matrix inverse();

        // Calculate the transport of this matrix
        void transpose(Matrix & result);

        // Print out the matrix
        void print() const;

        float* getMatrix();

        void getdMatrix3(dMatrix3 & result);

        int order;

    private:
        float * _matrix;
};

// Multiply vector * matrix
// NOTE: Assumes square matrix of same order as vector
inline Vector operator *(const Vector & v, const Matrix & m);
inline Vector operator *(const Matrix & m, const Vector & v);
inline Matrix operator*(const Matrix & m, const Matrix & m);
inline Matrix operator*(const Matrix & m, const float s);

/******************************************************************************
 * Inline definitions
 *****************************************************************************/

// TODO: i'm not sure these are right...
inline Vector operator *(const Vector & v, const Matrix & m) {
    Vector result(v.order);
    float sum = 0;
    for (int i = 0; i < v.order; ++i) {
        sum = 0;
        for (int j = 0; j < v.order; ++j) {
            sum += v[j] * m[i * m.order + j];
        }
        result[i] = sum;
    }
    return result;
}

inline Vector operator *(const Matrix & m, const Vector & v) {
    Vector result(v.order);
    float sum;
    for (int i = 0; i < v.order; ++i) {
        sum = 0;
        for (int j = 0; j < v.order; ++j) {
            sum += m[j * m.order + i] * v[j];
        }
        result[i] = sum;
    }
    return result;
}

inline Matrix operator*(const Matrix & a, const Matrix & b) {
    int order = min(a.order, b.order);
    Matrix result = Matrix(order);
    float sum;

    for (int i = 0; i < order; ++i) {
        for (int j = 0; j < order; ++j) {
            sum = 0;
            for (int k = 0; k < order; ++k) {
                sum += a[i + k * a.order] * b[j * b.order + k];
            }
            result[j * order + i] = sum;
        }
    }
    return result;
}

inline Matrix operator*(const Matrix & m, const float s) {
    Matrix result = m;
    for (int i = 0; i < m.order * m.order; ++i) {
        result[0] *= s;
    }
    return result;
}
