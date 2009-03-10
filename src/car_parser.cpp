#include "car_parser.h"
#include "logger.h"
#include "lib.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

Car * parseCar(string carPathString) {
    path carPath(carPathString);
    // A car is mostly described in a car.ini file, so we check it exists and open it
    if (!exists(carPath / "car.ini")) {
        Logger::warn << "Car model was not found: " << carPath / "car.ini" << endl;
        return NULL;
    }

    ifstream file((carPath / "car.ini").string().c_str());

    if (!file.is_open()) {
        Logger::warn << "Error opening file: " << carPath / "car.ini" << endl;
        return NULL;
    }

    // Now we actually parse the car.ini file. We build up a key value map, where the key
    // is the current path of the node in the ini file. I.e. object { model { file=x } } 
    // gets a key which is /object/model/file and the value is x, that way we can query
    // the map when we are creating the actual car object;
    map<string, string> inis;
    parseIniFile(file, inis);

    // Now we have the ini settings, we can load the actual car
    Dof * dof;
    Car * car = new Car();

    // Load the car body
    Logger::debug << "Loading the car..." << endl;
    if (inis.count("/body/model/file") > 0) {
        dof = new Dof((carPath / inis["/body/model/file"]).string(), 0);
        car->setBody(dof);
    }
    Logger::debug << "done loading the car" << endl;

    // Load the wheels
    Wheel * wheel;
    stringstream s;
    string p;
    float center[3];
    for (int i = 0; i < 4; ++i) {
        s << "/wheel" << i << "/model/file";
        if (inis.count(s.str()) > 0) {
            p = (carPath / inis[s.str()]).string();
            dof = new Dof(p, 0);
            wheel = new Wheel(i, dof);
            car->setWheel(wheel, i);
        }
        s.str("");

        // Try and get the center of the wheel from the suspension
        s << "/susp" << i << "/x";
        center[0] = atof(inis[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/y";
        center[1] = atof(inis[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/z";
        center[2] = atof(inis[s.str()].c_str());
        s.str("");

        // Add the roll center
        s << "/susp" << i << "/roll_center/x";
        center[0] += atof(inis[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/roll_center/y";
        center[1] += atof(inis[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/roll_center/z";
        center[2] += atof(inis[s.str()].c_str());
        s.str("");

        wheel->setCenter(center);
    }

    return car;
}

void parseIniFile(ifstream & file, map<string, string> & map) {
    vector<string> parts;
    list<string> currentPath;
    string path;
    string line;
    string previous;
    // Keep track of brackets so we know when the next token is a path token rather
    // than a value
    while (!file.eof()) {
        getline(file, line);
        trim(line);

        // Skip the line if its empty or is a comment
        if (line.size() == 0 || line[0] == ';') continue;

        // Check if we need to pop a path token, NOTE due to bugs in inis we ignore
        // too many closing brackets
        if (line[0] == '}' && currentPath.size() > 0) {
            currentPath.pop_back();
            continue;
        }

        // Check if the next value is a token
        if (line[0] == '{') {
            currentPath.push_back(previous);
            continue;
        }

        // Check if we have a key/value pair
        split(parts, line, is_any_of("="));
        previous = parts[0];
        if (parts.size() >= 2) {
            // build the current path
            path = makeIniPath(currentPath) + parts[0];
            map[path] = parts[1];
            continue;
        }
    }
}

string makeIniPath(list<string> & nodes) {
    list<string>::iterator it;
    string result = "/";
    for (it = nodes.begin(); it != nodes.end(); ++it) {
        result += *it;
        result += "/";
    }
    return result;
}
