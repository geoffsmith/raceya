#include "vector.h"
#include <iostream>
#include <math.h>

using namespace std;

Vector::Vector(const Vector & other) {
    this->order = other.order;
    this->_vector = new float[this->order];
    for (int i = 0; i < this->order; ++i) {
        this->_vector[i] = other[i];
    }
}

Vector & Vector::operator=(const Vector & other) {
    delete [] this->_vector;
    this->order = other.order;
    this->_vector = new float[this->order];
    for (int i = 0; i < this->order; ++i) {
        this->_vector[i] = other[i];
    }
    return *this;
}

Vector::Vector(float x, float y, float z) {
    this->order = 3;
    this->_vector = new float[this->order];
    this->_vector[0] = x;
    this->_vector[1] = y;
    this->_vector[2] = z;
}

Vector::Vector(float * vector, int size) {
    this->order = size;
    this->_vector = new float[this->order];
    for (int i = 0; i < this->order; ++i) {
        this->_vector[i] = vector[i];
    }
}

Vector::Vector(int order) {
    this->order = order;
    this->_vector = new float[this->order];
    for (int i = 0; i < this->order; ++i) {
        this->_vector[i] = 0;
    }
}

Vector::Vector() {
    this->order = 0;
    this->_vector = new float[this->order];
    for (int i = 0; i < this->order; ++i) {
        this->_vector[i] = 0;
    }
}

Vector::~Vector() {
    if (this->_vector != NULL) {
        delete [] (this->_vector);
        this->_vector = NULL;
    }
}

float Vector::magnitude() {
    float result = 0;
    for (int i = 0; i < this->order; ++i) {
        result += this->_vector[i] * this->_vector[i];
    }
    return sqrt(result);
}

float & Vector::operator [](int i) {
    return this->_vector[i];
}

float Vector::operator [](int i) const {
    return this->_vector[i];
}

Vector & Vector::operator+=(const Vector & rhs) {
    for (int i = 0; i < this->order; ++i) {
        this->_vector[i] += rhs[i];
    }
    return *this;
}

Vector & Vector::operator*=(const float scalar) {
    for (int i = 0; i < this->order; ++i) {
        this->_vector[i] *= scalar;
    }
    return *this;
}

void Vector::print() const {
    cout << "{ ";
    for (int i = 0; i < this->order; ++i) {
        cout << this->_vector[i] << ", ";
    }
    cout << "}" << endl;
}


