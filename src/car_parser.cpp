#include "car_parser.h"
#include "logger.h"
#include "lib.h"
#include "ini.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
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
    Ini carIniFile((carPath / "car.ini").string());

    // Now we have the ini settings, we can load the actual car
    Dof * dof;
    Car * car = new Car();
    string tmp;
    vector<string> parts;
    float center[3];

    // First we try and load the car shader
    //Shader::parseShaderFile((carPath / "car.shd").string());

    // Load the car body
    cout << "Loading the car...";
    if (carIniFile.hasKey("/body/model/file")) {
        cout << (carPath / carIniFile["/body/model/file"]).string() << endl;
        dof = new Dof((carPath / carIniFile["/body/model/file"]).string(), 0, false);
        car->setBody(dof);
    }
    cout << "done loading the car" << endl;
    
    // Load the car center of gravity
    if (carIniFile.hasKey("/aero/body/center")) {
        tmp = carIniFile["/aero/body/center"];
        split(parts, tmp, is_any_of(" "));
        center[0] = atof(parts[0].c_str());
        center[1] = atof(parts[1].c_str());
        center[2] = atof(parts[2].c_str());
        car->setCenter(center);
    } 

    // Load the wheels
    Wheel * wheel;
    stringstream s;
    string p;
    for (int i = 0; i < 4; ++i) {
        s << "/wheel" << i << "/model/file";
        if (carIniFile.hasKey(s.str())) {
            p = (carPath / carIniFile[s.str()]).string();
            dof = new Dof(p, 0, false);
            wheel = new Wheel(i, dof);
            car->setWheel(wheel, i);
        }
        s.str("");

        // Load the brake dof
        s << "/wheel" << i << "/model_brake/file";
        if (carIniFile.hasKey(s.str())) {
            p = (carPath / carIniFile[s.str()]).string();
            dof = new Dof(p, 0, false);
            wheel->setBrakeDof(dof);
        }
        s.str("");

        // Try and get the center of the wheel from the suspension
        s << "/susp" << i << "/x";
        center[0] = atof(carIniFile[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/y";
        center[1] = atof(carIniFile[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/z";
        center[2] = atof(carIniFile[s.str()].c_str());
        s.str("");

        // Add the roll center
        s << "/susp" << i << "/roll_center/x";
        center[0] += atof(carIniFile[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/roll_center/y";
        center[1] += atof(carIniFile[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/roll_center/z";
        center[2] += atof(carIniFile[s.str()].c_str());
        s.str("");

        wheel->setCenter(center);
    }

    return car;
}
