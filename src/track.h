/**
 * Class to load and render a racer format track. See http://racer.nl for details.
 *
 * Possible optimisations:
 *  * Cache transparent and non-transparent dofs
 *  * Use ini parser for configuration files
 */
#pragma once

#include <string>
#include <ode/ode.h>

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

        // Start position
        float startPosition[3];

        // The ODE world ID
        static dWorldID worldId;
        static dSpaceID spaceId;
        dGeomID planeId;

    private:
        string _path;

        //Dof ** _dofs;
        //unsigned int _nDofs;
        ptr_vector<Dof> dofs;

        // Load the geometry.ini file which points to the globs.
        // NOTE: this is ultra simplified at the moment and will almost certainly need 
        // expanding. It just looks for lines with a dof file and loads it.
        void _loadGeometryIni();

        // Load the special ini file which includes: grid positions, camera positions, 
        // high level track variables (sun position and colour)
        void Track::_loadSpecialIni();

        // Initialise the collision detection, this uses the built in ODE collision 
        // detection for now. 
        void Track::_initCollisionDetection();
};
