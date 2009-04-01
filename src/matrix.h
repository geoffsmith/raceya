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

#include <OpenGL/gl.h>

class Matrix {
    public:
        Matrix();
        Matrix(const Matrix & other);
        Matrix(GLfloat * input);
        Matrix(int order);
        ~Matrix();
        void reset();
        float& operator[] (const unsigned int index);
        float operator[] (const unsigned int index) const;
        Matrix & operator=(const Matrix & other);
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

        // Calculate the transport of this matrix
        void transpose(Matrix & result);

        // Print out the matrix
        void print();

        float* getMatrix();

        int order;

    private:
        float * _matrix;
};

// Multiply vector * matrix
// NOTE: Assumes square matrix of same order as vector
inline Vector operator *(const Vector & v, const Matrix & m);
inline Vector operator *(const Matrix & m, const Vector & v);

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
            sum += v[j] * m[j * m.order + i];
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
