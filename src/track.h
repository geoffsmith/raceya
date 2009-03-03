/**
 * Class to load and render a racer format track. See http://racer.nl for details.
 *
 * Possible optimisations:
 *  * Cache transparent and non-transparent dofs
 */
#pragma once

#include <string>
#include "dof.h"

using namespace std;

class Track {
    public:
        // Load up a track using the files in the given path
        Track(string path);
        ~Track();
        
        // Render a track
        void render();

        // Getters
        Dof ** getDofs() { return this->_dofs; }
        unsigned int getNDofs() { return this->_nDofs; }

    private:
        string _path;
        Dof ** _dofs;
        unsigned int _nDofs;

        // Load the geometry.ini file which points to the globs.
        // NOTE: this is ultra simplified at the moment and will almost certainly need 
        // expanding. It just looks for lines with a dof file and loads it.
        void _loadGeometryIni();
};
