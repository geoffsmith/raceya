/**
 * A collection of functions to parse a racer car into a car model
 */
#pragma once

#include <string>
#include <boost/filesystem.hpp>

#include "car.h"
#include "ini.h"

using namespace std;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

Car * parseCar(string path);
void parseEngine(Ini & ini, Car * car, path carPath);
