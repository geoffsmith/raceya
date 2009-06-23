/*
 * TODO:
 * * Refactor to use ini convenience methods
*/
#include "car_parser.h"
#include "logger.h"
#include "lib.h"
//#include "drive_systems.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

class Engine;
class Gearbox;

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
    float inertia[3];
    float restLength;

    // First we try and load the car shader
    Shader::parseShaderFile((carPath / "car.shd").string());

    // Load the car body
    if (carIniFile.hasKey("/body/model/file")) {
        cout << (carPath / carIniFile["/body/model/file"]).string() << endl;
        dof = new Dof((carPath / carIniFile["/body/model/file"]).string(), 0, false);
        car->setBody(dof);
    }

    // Load the brake model
    if (carIniFile.hasKey("/body/model_braking_l/file")) {
        dof = new Dof(
                (carPath / carIniFile["/body/model_braking_l/file"]).string(), 0, false);
        car->setBrakeModel(dof);
    }
    
    // Load the car center of gravity
    if (carIniFile.hasKey("/aero/body/center")) {
        tmp = carIniFile["/aero/body/center"];
        split(parts, tmp, is_any_of(" "));
        center[0] = atof(parts[0].c_str());
        center[1] = atof(parts[1].c_str());
        center[2] = atof(parts[2].c_str());
        car->setCenter(center);
    } 


    // Load the inertia
    if (carIniFile.hasKey("/body/inertia/x")) {
        tmp = carIniFile["/body/inertia/x"];
        inertia[0] = atof(tmp.c_str());
        tmp = carIniFile["/body/inertia/y"];
        inertia[1] = atof(tmp.c_str());
        tmp = carIniFile["/body/inertia/z"];
        inertia[2] = atof(tmp.c_str());
        //car->setInertia(inertia);
    } 

    // Load the mass
    if (carIniFile.hasKey("/body/mass")) {
        tmp = carIniFile["/body/mass"];
        car->setMass(atof(tmp.c_str()), inertia);
    }


    // Load the body surface area
    if (carIniFile.hasKey("/aero/body/area")) {
        tmp = carIniFile["/aero/body/area"];
        car->setBodyArea(atof(tmp.c_str()));
    }

    // Load the drag coefficient
    if (carIniFile.hasKey("/aero/body/cx")) {
        tmp = carIniFile["/aero/body/cx"];
        car->setDragCoefficient(atof(tmp.c_str()));
    }

    car->setDimensions(
        carIniFile.getFloat("/body/height"),
        carIniFile.getFloat("/body/width"),
        carIniFile.getFloat("/body/length")
    );

    // Load the wheels
    Wheel * wheel;
    stringstream s;
    string p;
    for (int i = 0; i < 4; ++i) {
        s << "/wheel" << i << "/model/file";
        if (carIniFile.hasKey(s.str())) {
            p = (carPath / carIniFile[s.str()]).string();
            dof = new Dof(p, 0, false);
            wheel = new Wheel(i, dof, *car);
            car->setWheel(wheel, i);
        } else {
            cout << "Wheel not found: " << s.str() << endl;
            exit(1);
        }

        // Get the wheel radius
        wheel->setRadius(carIniFile.getFloat("/wheel", i, "/radius"));

        // Set the mass and inertia
        wheel->setMass(
                carIniFile.getFloat("/wheel", i, "/mass"),
                carIniFile.getFloat("/wheel", i, "/inertia")
                );

        s.str("");

        // Find out if this wheel steers
        s << "/wheel" << i << "/steering";
        if (carIniFile[s.str()] == "1") {
            wheel->enableSteering();
        }

        // Load the brake dof
        s << "/wheel" << i << "/model_brake/file";
        if (carIniFile.hasKey(s.str())) {
            p = (carPath / carIniFile[s.str()]).string();
            dof = new Dof(p, 0, false);
            wheel->setBrakeDof(dof);
        }
        s.str("");

        // Check if this wheel is powered
        s << "/wheel" << i << "/powered";
        if (carIniFile[s.str()] == "1") {
            wheel->isPowered = true;
        }
        s.str("");

        // Try and get the center of the wheel from the suspension
        // TODO: refactor this to use the nice ini getters
        s << "/susp" << i << "/x";
        center[0] = atof(carIniFile[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/y";
        center[1] = atof(carIniFile[s.str()].c_str());
        s.str("");

        s << "/susp" << i << "/z";
        center[2] = atof(carIniFile[s.str()].c_str());
        s.str("");

        // Get the rest len and move the wheel in the (0, -1, 0) direction
        s << "/susp" << i << "/restlen";
        restLength = atof(carIniFile[s.str()].c_str());
        s.str("");

        center[1] -= restLength;

        // Set the max brake torque
        wheel->setMaxBrakeTorque(carIniFile.getFloat("/wheel", i, "/max_braking"));

        // Get the wheel rolling coefficient
        wheel->setRollingCoefficient(carIniFile.getFloat("/wheel", i, "/rolling_coeff"));

        // Get the lateral pacejka constants
        wheel->setLateralPacejka(
                carIniFile.getFloat("/pacejka/a0", 0),
                carIniFile.getFloat("/pacejka/a1", 0),
                carIniFile.getFloat("/pacejka/a2", 0),
                carIniFile.getFloat("/pacejka/a3", 0),
                carIniFile.getFloat("/pacejka/a4", 0),
                carIniFile.getFloat("/pacejka/a5", 0),
                carIniFile.getFloat("/pacejka/a6", 0),
                carIniFile.getFloat("/pacejka/a7", 0),
                carIniFile.getFloat("/pacejka/a8", 0),
                carIniFile.getFloat("/pacejka/a9", 0),
                carIniFile.getFloat("/pacejka/a10", 0),
                carIniFile.getFloat("/pacejka/a111", 0),
                carIniFile.getFloat("/pacejka/a12", 0),
                carIniFile.getFloat("/pacejka/a13", 0),
                carIniFile.getFloat("/pacejka/a112", 0)
        );

        wheel->setLongPacejka(
                carIniFile.getFloat("/pacejka/b0", 0),
                carIniFile.getFloat("/pacejka/b1", 0),
                carIniFile.getFloat("/pacejka/b2", 0),
                carIniFile.getFloat("/pacejka/b3", 0),
                carIniFile.getFloat("/pacejka/b4", 0),
                carIniFile.getFloat("/pacejka/b5", 0),
                carIniFile.getFloat("/pacejka/b6", 0),
                carIniFile.getFloat("/pacejka/b7", 0),
                carIniFile.getFloat("/pacejka/b8", 0),
                carIniFile.getFloat("/pacejka/b9", 0),
                carIniFile.getFloat("/pacejka/b10", 0),
                carIniFile.getFloat("/pacejka/b11", 0),
                carIniFile.getFloat("/pacejka/b12", 0)
        );

        wheel->setCenter(center);
    }

    parseEngine(carIniFile, car, carPath);

    return car;
}

void parseEngine(Ini & ini, Car * car, path carPath) {
    Engine & engine = car->getEngine();

    // Get the engine variables
    engine.setMass(ini.getFloat("/engine/mass"));
    engine.setRpm(
            ini.getFloat("/engine/max_rpm"),
            ini.getFloat("/engine/idle_rpm"),
            ini.getFloat("/engine/stall_rpm"),
            ini.getFloat("/engine/start_rpm") );
    engine.setDifferential(ini.getFloat("/differential/ratio"));

    // Get the torque curve
    Curve curve((carPath / ini["/engine/curve_torque"]).string());
    engine.setTorqueCurve(
            curve,
            ini.getFloat("/engine/max_torque"));

    engine.print();

    // Set up the gearbox
    Gearbox & gearbox = car->getGearbox();
    int nGears = ini.getInt("/gearbox/gears");
    gearbox.setNGears(nGears);
    for (int i = 0; i < nGears; ++i) {
        float ratio = ini.getFloat("/gearbox/gear", i, "/ratio");
        gearbox.setGearRatio(i, ratio);
    }

    gearbox.setShiftRpms(
            ini.getFloat("/engine/shifting/shift_up_rpm"),
            ini.getFloat("/engine/shifting/shift_down_rpm"));
}
