/**
 * A vector of floats class
 */
#pragma once

class Vector {
    public:
        // We need to be able to copy properly
        Vector(const Vector & other);
        Vector & operator=(const Vector & other);

        // Create an order 3 vector
        Vector(float x, float y, float z);

        // Create a vector from array of floats of size
        Vector(float * vector, int size);

        // Create a blank vector with order
        Vector(int order);

        // Get an index from a vector
        // NOTE: We don't check the order for speed
        float & operator [](int i);

        // We need a const version for the operator functions
        float operator [](int i) const;

        Vector & operator/=(float s);
        Vector & operator+=(const Vector & rhs);

        // Get the vector's magnitude
        float magnitude();

        // Print out the vecotr
        void print();

        // dtor
        ~Vector();

        // The order of the vector
        int order;

    private:
        // The vector data
        float * _vector;
};

// Multiply a vector and a scalar
inline Vector operator *(const float s, const Vector & v);
inline Vector operator *(const Vector & v, const float s);

// Vector dot product
inline float operator *(const Vector & a, const Vector & b);

// Subtract two vectors
inline Vector operator -(const Vector & a, const Vector & b);

// Add two vectors
inline Vector operator +(const Vector & a, const Vector & b);

// Vector cross product
inline Vector operator ^(const Vector & a, const Vector & b);

// Divide vector by scalar
//inline Vector operator /(Vector & v, float s);
inline Vector operator /(const Vector & v, float s);


/******************************************************************************
 * Inline definitions
 *****************************************************************************/

inline Vector operator *(const float s, const Vector & v) {
    Vector result(v.order);
    for (int i = 0; i < v.order; ++i) {
        result[i] = s * v[i];
    }
    return result;
}

inline Vector operator *(const Vector & v, const float s) {
    Vector result(v.order);
    for (int i = 0; i < v.order; ++i) {
        result[i] = s * v[i];
    }
    return result;
}

inline float operator *(const Vector & a, const Vector & b) {
    float result = 0;
    for (int i = 0; i < a.order; ++i) {
        result += a[i] * b[i];
    }
    return result;
}

inline Vector operator -(const Vector & a, const Vector & b) {
    Vector result(a.order);
    for (int i = 0; i < a.order; ++i) {
        result[i] = a[i] - b[i];
    }
    return result;
}

inline Vector operator +(const Vector & a, const Vector & b) {
    Vector result(a.order);
    for (int i = 0; i < a.order; ++i) {
        result[i] = a[i] + b[i];
    }
    return result;
}

inline Vector operator ^(const Vector & a, const Vector & b) {
    // We're not supporting x product for anything other than 3 yet
    if (a.order != 3) {
        throw "Order not supported on cross product";
    }

    Vector result(a.order);
    result[0] =  a[1] * b[2] - b[1] * a[2];
    result[1] =  a[0] * b[2] - b[0] * a[2];
    result[2] =  a[0] * b[1] - b[0] * a[1];
    return result;
}

inline Vector & Vector::operator/=(float s) {
    for (int i = 0; i < this->order; ++i) {
        this->_vector[i] = s * this->_vector[i];
    }
    return *this;
}

inline Vector operator /(const Vector & v, float s) {
    Vector result(v.order);
    for (int i = 0; i < v.order; ++i) {
        result[i] = v[i] / s;
    }
    return result;
}
