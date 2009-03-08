#include "track.h"
#include "shader.h"
#include "closest_point.h"
#include "logger.h"

#include <OpenGL/gl.h>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

Track::Track(string trackPath) {
    path currentDir("./");
    this->_path = trackPath;

    // try to load shaders
    Shader::parseShaderFile((currentDir / trackPath / "track.shd").string().c_str());

    // Look for a geometry.ini file, and use this to load the dofs
    if (!exists(currentDir / trackPath / "geometry.ini")) {
        cout << "Warning: couldn't find geometry.ini" << endl;
        return;
    }

    this->_loadGeometryIni();
}

Track::~Track() {
    // Clean up the dof objects
    for (unsigned int i = 0; i < this->_nDofs; ++i) {
        delete this->_dofs[i];
    }
    delete[] this->_dofs;
}

void Track::_loadGeometryIni() { 
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
    ifstream file((currentDir / this->_path / "geometry.ini").string().c_str());
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
            dofFiles[currentObject] = 
                (currentDir / this->_path / parts[1]).string();
        } else if (parts[0] == "flags") {
            // Convert the flags to an int
            int f = atoi(parts[1].c_str());
            flags[currentObject] = f;
        }

    }

    file.close();

    // Load the dof files using the paths and flags
    vector<Dof *> dofs;
    Dof * tmpDof;
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
        tmpDof = new Dof(it->second, currentFlags);

        // Check that it loaded properly, if so add it to the list
        if (tmpDof->isValid) {
            dofs.push_back(tmpDof);
        } else {
            delete tmpDof;
        }
    }

    // Now copy this all into an old school array for speed
    this->_nDofs = dofs.size();
    this->_dofs = new Dof *[this->_nDofs];
    for (unsigned int i = 0; i < this->_nDofs; ++i) {
        this->_dofs[i] = dofs[i];
    }
}

void Track::render() {
    int count = 0;
    int total = 0;
    // We do this in two passes, first for non-transparent, then for transparent
    for (unsigned int i = 0; i < this->_nDofs; ++i) {
        if (!(this->_dofs[i]->isTransparent())) {
            count += this->_dofs[i]->render();
        }
        total++;
    }    

    for (unsigned int i = 0; i < this->_nDofs; ++i) {
        if (this->_dofs[i]->isTransparent()) {
            count += this->_dofs[i]->render();
        }
        total++;
    }    
}
