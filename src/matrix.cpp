#include "matrix.h"
#include <math.h>
#include <iostream>

#define PI 3.14159265

using namespace std;

Matrix::Matrix() {
    // Start with the identity matrix
    this->_matrix = new float[16];
    this->reset();
}

Matrix::Matrix(GLfloat *input) {
    // Start with the identity matrix
    this->_matrix = new float[16];
    for (int i = 0; i < 16; ++i) {
        this->_matrix[i] = input[i];
    }
}

void Matrix::reset() {
    this->_matrix[0] = 1;
    this->_matrix[1] = 0;
    this->_matrix[2] = 0;
    this->_matrix[3] = 0;

    this->_matrix[4] = 0;
    this->_matrix[5] = 1;
    this->_matrix[6] = 0;
    this->_matrix[7] = 0;

    this->_matrix[8] = 0;
    this->_matrix[9] = 0;
    this->_matrix[10] = 1;
    this->_matrix[11] = 0;

    this->_matrix[12] = 0;
    this->_matrix[13] = 0;
    this->_matrix[14] = 0;
    this->_matrix[15] = 1;
}

Matrix::~Matrix() {
    delete[] this->_matrix;
}

void Matrix::rotate(float angle, float * normal) {
    float radians = angle * PI / 180.0;
    float c = cos(radians);
    float s = sin(radians);

    // Normalised normal
    float n[3];
    float d;

    n[0] = normal[0];
    n[1] = normal[1];
    n[2] = normal[2];
    
    d = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    // .. normalise so that x2 + y2 + z2 = 1
    n[0] /= d;
    n[1] /= d;
    n[2] /= d;
    
    Matrix rotation;
    rotation[0] = n[0] * n[0] + (1 - n[0] * n[0]) * c;
    rotation[1] = n[0] * n[1] + (1 - c) + (n[2] * s);
    rotation[2] = n[0] * n[2] + (1 - c) - (n[1] * s);
    rotation[3] = 0;

    rotation[4] = n[0] * n[1] + (1 - c) - (n[2] * s);
    rotation[5] = n[1] * n[1] + (1 - n[1] * n[1]) * c;
    rotation[6] = n[1] * n[2] + (1 - c) + (n[0] * s);
    rotation[7] = 0;

    rotation[8] = n[0] * n[2] + (1 - c) + (n[1] * s);
    rotation[9] = n[1] * n[2] + (1 - c) - (n[0] * s);
    rotation[10] = n[2] * n[2] + (1 - n[2] * n[2]) * c;
    rotation[11] = 0;

    rotation[12] = 0;
    rotation[13] = 0;
    rotation[14] = 0;
    rotation[15] = 1;

    this->multiplyMatrix(&rotation);
}

void Matrix::rotateY(float angle) {
    // Convert into radians
    float radians = angle * PI / 180.0;

    // Build the rotation matrix
    Matrix rotationMatrix;
    rotationMatrix[0] = cos(radians);
    rotationMatrix[1] = 0;
    rotationMatrix[2] = -1 * sin(radians);
    rotationMatrix[3] = 0;

    rotationMatrix[4] = 0;
    rotationMatrix[5] = 1;
    rotationMatrix[6] = 0;
    rotationMatrix[7] = 0;

    rotationMatrix[8] = sin(radians);
    rotationMatrix[9] = 0;
    rotationMatrix[10] = cos(radians);
    rotationMatrix[11] = 0;

    rotationMatrix[12] = 0;
    rotationMatrix[13] = 0;
    rotationMatrix[14] = 0;
    rotationMatrix[15] = 1;

    // Multiply it into the current matrix
    this->multiplyMatrix(&rotationMatrix);
}

void Matrix::rotateX(float angle) {
    // Convert into radians
    float radians = angle * PI / 180.0;

    // Build the rotation matrix
    Matrix rotationMatrix;
    rotationMatrix[0] = 1;
    rotationMatrix[1] = 0;
    rotationMatrix[2] = 0;
    rotationMatrix[3] = 0;

    rotationMatrix[4] = 0;
    rotationMatrix[5] = cos(radians);
    rotationMatrix[6] = sin(radians);
    rotationMatrix[7] = 0;

    rotationMatrix[8] = 0;
    rotationMatrix[9] = -1 * sin(radians);
    rotationMatrix[10] = cos(radians);
    rotationMatrix[11] = 0;

    rotationMatrix[12] = 0;
    rotationMatrix[13] = 0;
    rotationMatrix[14] = 0;
    rotationMatrix[15] = 1;

    // Multiply it into the current matrix
    this->multiplyMatrix(&rotationMatrix);
}

void Matrix::rotateZ(float angle) {
    // Convert into radians
    float radians = angle * PI / 180.0;

    // Build the rotation matrix
    Matrix rotationMatrix;
    rotationMatrix[0] = cos(radians);
    rotationMatrix[1] = sin(radians);

    rotationMatrix[4] = -1 * sin(radians);
    rotationMatrix[5] = cos(radians);

    // Multiply it into the current matrix
    this->multiplyMatrix(&rotationMatrix);
}

void Matrix::translate(float x, float y, float z) {
    Matrix translationMatrix;

    translationMatrix[12] = x;
    translationMatrix[13] = y;
    translationMatrix[14] = z;

    this->multiplyMatrix(&translationMatrix);
}

void Matrix::scale(float scale) {
    Matrix scaleMatrix;

    scaleMatrix[0] = scale;
    scaleMatrix[5] = scale;
    scaleMatrix[10] = scale;

    this->multiplyMatrix(&scaleMatrix);
}

void Matrix::multiplyVector(float *vector, float *result) {
    float *thisMatrix = this->_matrix;
    float sum;
    for (int i = 0; i < 4; ++i) {
        sum = 0;
        for (int j = 0; j < 4; ++j) {
            sum += thisMatrix[j * 4 + i] * vector[j];
        }
        result[i] = sum;
    }
}

void Matrix::multiplyVector(float *vector) {
    float *thisMatrix = this->_matrix;
    float sum;
    float tmp[4];
    float result[4];

    tmp[0] = vector[0];
    tmp[1] = vector[1];
    tmp[2] = vector[2];
    tmp[3] = 1;

    for (int i = 0; i < 4; ++i) {
        sum = 0;
        for (int j = 0; j < 4; ++j) {
            sum += thisMatrix[j * 4 + i] * tmp[j];
        }
        result[i] = sum;
    }

    vector[0] = result[0] / result[3];
    vector[1] = result[1] / result[3];
    vector[2] = result[2] / result[3];
}

void Matrix::multiplyVectorSkipTranslation(float *vector, float *result) {
    float *thisMatrix = this->_matrix;
    float sum;
    for (int i = 0; i < 4; ++i) {
        sum = 0;
        for (int j = 0; j < 4; ++j) {
            // If this is a translation item, skip
            if (!((j * 4 + i >= 12) && (j * 4 + i < 15))) {
                sum += thisMatrix[j * 4 + i] * vector[j];
            }
        }
        result[i] = sum;
    }
}

float& Matrix::operator[] (const unsigned int index) {
    return this->_matrix[index];
}

void Matrix::multiplyMatrix(float * matrix) {
    Matrix tmp(matrix);
    this->multiplyMatrix(&tmp);
}

void Matrix::multiplyMatrix(Matrix *matrix) {
    float *thisMatrix = this->_matrix;
    float sum;
    float *result = new float[16];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            sum = 0;
            for (int k = 0; k < 4; ++k) {
                sum +=  thisMatrix[k * 4 + i] * (*matrix)[j * 4 + k];
            }
            result[j * 4 + i] = sum;
        }
    }

    // Swap to the new matrix
    delete[] this->_matrix;
    this->_matrix = result;
}

float* Matrix::getMatrix() {
    return this->_matrix;
}
