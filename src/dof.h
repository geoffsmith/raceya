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

// Utility function to load vector
template <class T> void parseVector(ifstream * file, T * vector, int length);
// to load string
int parseString(ifstream * file, char * buffer);

class Geob {
    public:
        ~Geob();
        int material;
        int nIndices;
        unsigned short * indices;
        unsigned int nVertices;
        float (* vertices)[3];
        unsigned int nTextureCoords;
        float (* textureCoords)[2];
        unsigned int nNormals;
        float (* normals)[3];
        int nBursts;
        int * burstStarts;
        int * burstsCount;
        int * burstsMaterials;
        // Still to implement
        // * VCOL
        // * BRST
        // * INDI
};

class Mat {
    public:
        ~Mat();
        float ambient[4];
        float diffuse[4];
        float specular[4];
        float emission[4];
        float shininess;
        // UVW mapping information
        float uvwUoffset;
        float uvwVoffset;
        float uvwUtiling;
        float uvwVtiling;
        float uvwAngle;
        float uvwBlur;
        float uvwBlurOffset;

        int nTextures;
        unsigned int * textures;
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
        int _nGeobs;

        // Material objects
        Mat * _mats;
        int _nMats;

        // Parse the geobs from the file into Geob objects. This assumes that the file
        // pointer is at the start of GOB1 and will leave the pointer at the end of GEND
        void _parseGeobs(ifstream * file);

        // Parse the mats from the file into Mat objects. Like _parseGeobs, it assumes that
        // the file pointer is at the beginning of the first MAT0 and leaves it after MEND
        void _parseMats(ifstream * file);

        // Load a texture
        void _loadTexture(string name, unsigned int & texture);
};

