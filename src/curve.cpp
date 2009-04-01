#include "curve.h"
#include "ini.h"

#include <boost/format.hpp>
#include <iostream>


using namespace std;
using boost::format;

Curve::Curve() {
    this->_data = NULL;
}

Curve::Curve(string filename) {
    this->_filename = filename;
    this->_data = NULL;
    this->_parseFile();
}

Curve::Curve(const Curve & other) {
    this->_filename = other._filename.c_str();
    this->_max[0] = other._max[0];
    this->_max[1] = other._max[1];
    this->_min[0] = other._min[0];
    this->_min[1] = other._min[1];
    this->_dataLength = other._dataLength;
    cout << "Data: " << this->_dataLength << endl;
    this->_data = new float[this->_dataLength][2];
    for (int i = 0; i < this->_dataLength; ++i) {
        this->_data[i][0] = other._data[i][0];
        this->_data[i][1] = other._data[i][1];
    }
}

Curve & Curve::operator=(const Curve & other) {
    this->_filename = other._filename.c_str();
    this->_max[0] = other._max[0];
    this->_max[1] = other._max[1];
    this->_min[0] = other._min[0];
    this->_min[1] = other._min[1];
    this->_dataLength = other._dataLength;

    // Remove data if there alreayd is some
    if (this->_data != NULL) delete [] this->_data;

    this->_data = new float[this->_dataLength][2];
    for (int i = 0; i < this->_dataLength; ++i) {
        this->_data[i][0] = other._data[i][0];
        this->_data[i][1] = other._data[i][1];
    }
    return *this;
}

Curve::~Curve() {
    if (this->_data != NULL) delete [] this->_data;
}

void Curve::_parseFile() {
    Ini ini(this->_filename);

    // Get the max and min
    this->_max[0] = ini.getInt("/curve/xmax");
    this->_max[1] = ini.getInt("/curve/ymax");
    this->_min[0] = ini.getInt("/curve/xmin");
    this->_min[1] = ini.getInt("/curve/ymin");

    // Get the actual data points
    this->_dataLength = ini.getInt("/curve/points");
    this->_data = new float[this->_dataLength][2];

    // And now the nodes in-between
    for (int i = 0; i < this->_dataLength; ++i) {
        this->_data[i][0] = ini.getFloat(str(boost::format("/curve/point%1%/x") % i));
        this->_data[i][1] = ini.getFloat(str(boost::format("/curve/point%1%/y") % i));
    }
}

float Curve::operator[](float x) {
    float u;
    int i;

    // Make sure the point is not ouside the bounds, if so clamp it
    if (x >= this->_max[0]) return this->_min[1];
    if (x <= this->_min[0]) return this->_min[1];

    // Get the next highest after the given value
    for (i = 0; i < this->_dataLength; ++i) {
        if (x <= this->_data[i][0]) break;
    }

    // Check that i is not 0 or this->_dataLength - 1 because these are out of range
    if (i == 0 || i == this->_dataLength - 1) {
        return this->_min[1];
    }

    // Now we know the value is between i and i - 1, we interpolate
    u = (x - this->_data[i - 1][0]) / (this->_data[i][0] - this->_data[i - 1][0]);
    return u * this->_data[i - 1][1] + (1 - u) * this->_data[i][1];
}
