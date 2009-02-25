/**
 * Class for parsing and rendering DOF files
 *
 * Specification: http://racer.nl/dof.htm
 *
 * TODO:
 *  * Be able to load more than 1 DOF from a single file
 */
#pragma once

#include <string>

using namespace std;

class Geob {
    public:
        ~Geob();
        unsigned int nIndices;
        short * indices;
        unsigned int nVertices;
        float (* vertices)[3];
        unsigned int nTextureCoords;
        float (* textureCoords)[2];
        unsigned int nNormals;
        float (* normals)[3];
        // Still to implement
        // * VCOL
        // * BRST
};

class Dof {
    public:
        Dof(string filePath);
        ~Dof();
        void render();

    private:
        string _filePath;

        // Geometrical objects
        Geob * _geobs;
        unsigned int _nGeobs;

        // Parse the geobs from the file into Geob objects. This assumes that the file
        // pointer is at the start of GOB1 and will leave the pointer at the end of GEND
        void _parseGeobs(ifstream * file);
};
