#include "matrix.h"
#include <math.h>
#include <iostream>

#define PI 3.14159265

using namespace std;

Matrix::Matrix() {
    this->order = 4;
    // Our default matrix is order 4
    this->_matrix = new float[this->order * this->order];
    this->reset();
}

Matrix::Matrix(int order) {
    this->order = order;
    this->_matrix = new float[this->order * this->order];
    this->reset();
}

Matrix::Matrix(GLfloat *input) {
    // Start with the identity matrix
    this->order = 4;
    this->_matrix = new float[this->order * this->order];
    for (int i = 0; i < 16; ++i) {
        this->_matrix[i] = input[i];
    }
}

void Matrix::reset() {
    // Start with the identity matrix
    for (int i = 0; i < this->order; ++i) {
        for (int j = 0; j < this->order; ++j) {
            if (i == j) {
                this->_matrix[i * this->order + j] = 1;
            } else {
                this->_matrix[i * this->order + j] = 0;
            }
        }
    }
}

Matrix::~Matrix() {
    //delete[] this->_matrix;
}

void Matrix::rotate(float angle, float * normal) {
    float radians = angle * PI / 180.0;
    float c = cos(radians);
    float s = sin(radians);
    Matrix rotation;

    // Normalised normal
    float n[3];
    float d;

    n[0] = normal[0];
    n[1] = normal[1];
    n[2] = normal[2];
    
    d = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);

    // If d is 0, we need to stop
    if (d == 0.0) return;

    // .. normalise so that x2 + y2 + z2 = 1
    n[0] /= d;
    n[1] /= d;
    n[2] /= d;
    
    rotation[0] = n[0] * n[0] + (1 - n[0] * n[0]) * c;
    rotation[1] = n[0] * n[1] * (1 - c) + (n[2] * s);
    rotation[2] = n[0] * n[2] * (1 - c) - (n[1] * s);
    rotation[3] = 0;

    rotation[4] = n[0] * n[1] * (1 - c) - (n[2] * s);
    rotation[5] = n[1] * n[1] + (1 - n[1] * n[1]) * c;
    rotation[6] = n[1] * n[2] * (1 - c) + (n[0] * s);
    rotation[7] = 0;

    rotation[8] = n[0] * n[2] * (1 - c) + (n[1] * s);
    rotation[9] = n[1] * n[2] * (1 - c) - (n[0] * s);
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
    float * thisMatrix = this->_matrix;
    float sum;
    for (int i = 0; i < this->order; ++i) {
        sum = 0;
        for (int j = 0; j < this->order; ++j) {
            sum += thisMatrix[j * this->order + i] * vector[j];
        }
        result[i] = sum;
    }
}

void Matrix::multiplyVector(float *vector) {
    float *thisMatrix = this->_matrix;
    float sum;
    float tmp[this->order];
    float result[this->order];

    // TODO: this is horrible, need to sort out orders
    tmp[0] = vector[0];
    tmp[1] = vector[1];
    tmp[2] = vector[2];

    if (this->order == 4) {
        tmp[3] = 1;
    }

    for (int i = 0; i < this->order; ++i) {
        sum = 0;
        for (int j = 0; j < this->order; ++j) {
            sum += thisMatrix[j * this->order + i] * tmp[j];
        }
        result[i] = sum;
    }

    if (this->order == 4) {
        vector[0] = result[0] / result[3];
        vector[1] = result[1] / result[3];
        vector[2] = result[2] / result[3];
    } else {
        for (int i = 0; i < this->order; ++i) {
            vector[i] = result[i];
        }
    }
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

void Matrix::minor(Matrix & minor, int row, int column) {
    int p, q;

    p = q = 0;

    for (int i = 0; i < this->order; ++i) {
        if (i != row) {
            q = 0;
            for (int j = 0; j < this->order; ++j) {
                if (j != column) {
                    minor[q * minor.order + p] = this->_matrix[j * this->order + i];
                    ++q;
                }
            }
            ++p;
        }
    }
}

float Matrix::determinant() {
    float result = 0;
    float factor;

    // Stopping condition
    if (this->order < 1) {
        return 0;
    }

    // ... the usual stopping condition
    if (this->order == 1) {
        return this->_matrix[0];
    }

    if (this->order == 2) {
        return this->_matrix[0] * this->_matrix[3] - this->_matrix[1] * this->_matrix[2];
    }

    Matrix minor(this->order - 1);

    for (int i = 0; i < 1; ++i) {
        for (int j = 0; j < this->order; ++j) {
            this->minor(minor, i, j);
            factor = minor.determinant();
            result += pow(-1, i + j)  
                * this->_matrix[j * this->order + i] 
                * factor;
        }
    }

    return result;
}

void Matrix::invert(Matrix & result) {
    // Calculate the inverse of this matrix, using the adjoint method
    // First we need the determinant
    float determinant = this->determinant();
    determinant = 1.0 / this->determinant();

    Matrix minor(this->order - 1);

    for (int i = 0; i < this->order; ++i) {
        for (int j = 0; j < this->order; ++j) {
            this->minor(minor, j, i);
            result[j * this->order + i] = determinant * minor.determinant();
            if ((i + j) % 2 == 1) {
                result[j * this->order + i] = -result[j * this->order + i];
            }
        }
    }
}

void Matrix::transpose(Matrix & result) {
    for (int i = 0; i < this->order; ++i) {
        for (int j = 0; j < this->order; ++j) {
            result[j * this->order + i] = this->_matrix[i * this->order + j];
        }
    }
}

void Matrix::print() {
    for (int i = 0; i < this->order; ++i) {
        for (int j = 0; j < this->order; ++j) {
            cout << this->_matrix[j * this->order + i] << " ";
        }
        cout << endl;
    }
}
