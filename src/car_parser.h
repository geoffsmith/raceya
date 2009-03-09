/**
 * A collection of functions to parse a racer car into a car model
 */
#pragma once

#include <string>
#include <list>
#include <map>
#include "car.h"

using namespace std;

Car * parseCar(string path);

// Parse an ini file into a map
void parseIniFile(ifstream & file, map<string, string> & map);

// Build a slash separated string representing the nodes stack
string makeIniPath(list<string> & nodes);
