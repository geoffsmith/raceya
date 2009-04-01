/**
 * Class to parse in and allow access to a curve. I.e. for torque curves. The file will
 * describe a number of points and we will linearly interpolate between those points to
 * find the point we need.
 *
 * At the moment we're only going to implement an x-based lookup. Though it might turn out
 * that we also need y, which should effect the data structure quite a bit.
 *
 * TODO:
 * 	- Use non-linear interpolation
 */
#pragma once

#include <string>

using namespace std;

class Curve {
    public:
        Curve();
        Curve(string filename);
        Curve(const Curve & other);
        Curve & operator=(const Curve & other);
        ~Curve();

        // Estimate a point on the curve from the given x-value, we linearly interpolate
        float operator[](float x);

    private:
        // Parse the curve file
        void _parseFile();

        // The file containing this curve's definition
        string _filename;

        // The curve data is an array of 2 floats
        int _dataLength;
        float (* _data)[2];
        float _max[2];
        float _min[2];
};
