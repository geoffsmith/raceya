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
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "dof.h"

using namespace std;

class Track {
    public:
        // Load up a track using the files in the given path
        Track(string path);
        ~Track();
        
        // Render a track
        void render();

        // Start position
        float startPosition[3];

        // The ODE world ID
        static dWorldID worldId;
        static dSpaceID spaceId;
        dGeomID planeId;

    private:
        string iniPath;

        // List of dof objects which make up the track model
        boost::ptr_list<Dof> dofs;

        // Load the geometry.ini file which points to the globs.
        // NOTE: this is ultra simplified at the moment and will almost certainly need 
        // expanding. It just looks for lines with a dof file and loads it.
        void loadGeometryIni();

        // Load the special ini file which includes: grid positions, camera positions, 
        // high level track variables (sun position and colour)
        void loadSpecialIni();

        // Initialise the collision detection, this uses the built in ODE collision 
        // detection for now. 
        void initCollisionDetection();
};
