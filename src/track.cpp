#include "track.h"
#include "shader.h"
#include "closest_point.h"
#include "logger.h"

#include <GL/gl.h>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <vector>
#include <fstream>

using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

dWorldID Track::worldId;
dSpaceID Track::spaceId;

Track::Track(string trackPath) {
    path currentDir("./");
    this->iniPath = trackPath;

    // try to load shaders
    Shader::parseShaderFile((currentDir / trackPath / "track.shd").string().c_str());

    // Look for a geometry.ini file, and use this to load the dofs
    if (!exists(currentDir / trackPath / "geometry.ini")) {
        cout << "Warning: couldn't find geometry.ini" << endl;
        return;
    }

    this->loadGeometryIni();

    // Load and parse special.ini
    this->loadSpecialIni();

    dInitODE();

    // Create an ODE world for the physics of this track and its objects
    Track::worldId = dWorldCreate();

    // We set a normal gravity, the ODE default is 0
    dWorldSetGravity(Track::worldId, 0, -9.8, 0);

    Track::spaceId = dHashSpaceCreate(0);

    this->initCollisionDetection();
}

Track::~Track() {
    // Clean up ODE physics
    dWorldDestroy(Track::worldId);
}

void Track::initCollisionDetection() {
    // For each dof
    BOOST_FOREACH(Dof & dof, this->dofs) {
        // Check that this dof is collision material
        if (!dof.isSurface()) {
            continue;
        }

        // For each geob
        BOOST_FOREACH(Geob & geob, dof.getGeobs()) {
            dTriMeshDataID meshId = dGeomTriMeshDataCreate();

            // Convert geob vertices into dVector3s
            dReal * vertices = new dReal[geob.nVertices * 4];
            for (unsigned int k = 0; k < geob.nVertices; ++k) {
                vertices[k * 4] = geob.vertices[k][0];
                vertices[k * 4 + 1] = geob.vertices[k][1];
                vertices[k * 4 + 2] = geob.vertices[k][2];
                vertices[k * 4 + 3] = 1;
            }

            int * indices = new int[geob.nIndices];
            for (int k = 0; k < geob.nIndices; ++k) {
                indices[k] = geob.indices[k];
            }

            // Create the collision detection objects
            dGeomTriMeshDataBuildSimple(meshId, vertices, geob.nVertices,
                    indices, geob.nIndices);

            // Create the geob
            dCreateTriMesh(Track::spaceId, meshId, NULL, NULL, NULL);
            
            // TODO: epic memory leak here, we need to clean up all those indices and
            // vertices once ODE has finished with them
        }
    }
}

void Track::loadSpecialIni() {
    string value;
    path currentDir("./");
    Ini ini((currentDir / this->iniPath / "special.ini").string());

    // Get the starting position
    value = ini["/grid/pos0/from/x"];
    this->startPosition[0] = atof(value.c_str());
    value = ini["/grid/pos0/from/y"];
    this->startPosition[1] = atof(value.c_str());
    value = ini["/grid/pos0/from/z"];
    this->startPosition[2] = atof(value.c_str());
}

void Track::loadGeometryIni() { 
    bool nextTagIsObjectName = true;
    bool foundObjects = false;
    string currentObject;
    string line;
    vector<string> parts;
    vector<string> filenameParts;
    //vector<string> dofFiles;
    map<string, int> flags;
    map<string, string> dofFiles;
    path currentDir("./");
    ifstream file((currentDir / this->iniPath / "geometry.ini").string().c_str());
    if (!file.is_open()) {
        Logger::debug << "Error opening geometry.ini" << endl;
        return;
    }

    // Look for objects definition, basically get us to the point "object {"
    while (!file.eof()) {
        getline(file, line);
        trim(line);

        if (line == "objects") foundObjects = true;
        else if (line == "{" && foundObjects) break;
    }

    // look for dofs
    while (!file.eof()) {
        getline(file, line);

        // Strip the whitespace from the line
        trim(line);

        // if this is not a bracket and the next tag is the object name, set it
        if (nextTagIsObjectName && line.size() > 0 && line != "{" && line != "}") {
            nextTagIsObjectName = false;
            currentObject = line;
            continue;
        }

        // If the next tag is not supposed to be a name and we find a closing bracket,
        // the next tag will be an object name
        if (!nextTagIsObjectName && line == "}") {
            nextTagIsObjectName = true;
            continue;
        }

        // attempt to split by the = sign
        split(parts, line, is_any_of("="));

        // if we don't have two parts, move on
        if (parts.size() < 2) continue;

        // Check the command
        if (parts[0] == "file") {
            // split the second part and check for a dof extension
            split(filenameParts, parts[1], is_any_of("."));
            if (filenameParts.size() < 2) continue;
            if (filenameParts[1] != "dof") continue;

            // Load this file because it is a DOF file
            dofFiles[currentObject] = (currentDir / this->iniPath / parts[1]).string();
        } else if (parts[0] == "flags") {
            // Convert the flags to an int
            int f = atoi(parts[1].c_str());
            flags[currentObject] = f;
        }

    }

    file.close();

    // Load the dof files using the paths and flags
    map<string, string>::iterator it;
    int currentFlags;
    for (it = dofFiles.begin(); it != dofFiles.end(); ++it) {
        // Check to see if there are any flags
        if (flags.count(it->first) > 0) {
            currentFlags = flags[it->first];
        } else {
            currentFlags = 0;
        }

        // Attempt to create a dof
        Dof * tmpDof = new Dof(it->second, currentFlags);

        tmpDof->isTransparent();

        // Check that it loaded properly, if so add it to the list
        if (tmpDof->isValid) {
            this->dofs.push_back(tmpDof);
        } else {
            delete tmpDof;
        }
    }
}

void Track::render() {
    // We do this in two passes, first for non-transparent, then for transparent
    BOOST_FOREACH(Dof & dof, this->dofs) {
        if (!dof.isTransparent()) dof.render();
    }    

    BOOST_FOREACH(Dof & dof, this->dofs) {
        if (dof.isTransparent()) dof.render();
    }    
}
