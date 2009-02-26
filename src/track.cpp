#include "track.h"

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
    string line;
    vector<string> parts;
    vector<string> filenameParts;
    vector<string> dofFiles;
    path currentDir("./");
    ifstream file((currentDir / this->_path / "geometry.ini").string().c_str());
    if (!file.is_open()) {
        cout << "Error opening geometry.ini" << endl;
        return;
    }

    // look for dofs
    cout << "Reading geometry.ini" << endl;
    while (!file.eof()) {
        getline(file, line);

        // Strip the whitespace from the line
        trim(line);

        // attempt to split by the = sign
        split(parts, line, is_any_of("="));

        // if we don't have two parts, move on
        if (parts.size() < 2) continue;

        // split the second part and check for a dof extension
        split(filenameParts, parts[1], is_any_of("."));
        if (filenameParts.size() < 2) continue;
        if (filenameParts[1] != "dof") continue;

        // Load this file because it is a DOF file
        dofFiles.push_back((currentDir / this->_path / parts[1]).string());
    }

    file.close();

    // Now we have a list of dof files, we load them into an array
    this->_nDofs = dofFiles.size();
    this->_dofs = new Dof *[this->_nDofs];

    cout << "Load DOFs" << endl;
    for (unsigned int i = 0; i < this->_nDofs; ++i) {
        this->_dofs[i] = new Dof(dofFiles[i]);
    }
}

void Track::render() {
    int count = 0;
    for (unsigned int i = 0; i < this->_nDofs; ++i) {
        count += this->_dofs[i]->render();
    }    
}
