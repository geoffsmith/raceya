/**
 * Class for parsing and rendering DOF files
 *
 * Specification: http://racer.nl/dof.htm
 *
 * TODO:
 *  * Be able to load more than 1 DOF from a single file
 *  * Remove _loadTexture in favour of version in lib
 *  * Sort out the texture mapping properly, at the moment we are manyally flipping the y
 */
#pragma once

#include <string>
#include "shader.h"

using namespace std;

// Utility function to load vector
template <class T> void parseVector(ifstream * file, T * vector, int length);
// to load string
int parseString(ifstream * file, char * buffer);

// Define the geometry object flags
#define DOF_COLLISION 2
#define DOF_SURFACE 4

class Geob {
    public:
        Geob();
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
        unsigned int displayList;
        float boundingBox[6];
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
        // 0: no blending, 1: source alpha blending
        int blendMode;

        int nTextures;
        unsigned int * textures;
        // Pointer to some shaders, this array will be the same size as textures
        // but may have NULL entries
        Shader ** shaders;

        Geob * getGeob(int index);
        int getNGeobs();
};

class Dof {
    public:
        Dof(string filePath, int flags);
        ~Dof();
        int render();

        // Return true if one of the materials is transparent
        bool isTransparent();

        Geob * getGeob(unsigned int index);
        int getNGeobs();

        bool isValid; 

        // Return true if this is part of the track surface
        bool isSurface();

    private:
        string _filePath;

        // Geometrical objects
        Geob * _geobs;
        int _nGeobs;

        // Material objects
        Mat * _mats;
        int _nMats;

        // Flags
        int _flags;

        // Parse the geobs from the file into Geob objects. This assumes that the file
        // pointer is at the start of GOB1 and will leave the pointer at the end of GEND
        void _parseGeobs(ifstream * file);

        // Parse the mats from the file into Mat objects. Like _parseGeobs, it assumes that
        // the file pointer is at the beginning of the first MAT0 and leaves it after MEND
        void _parseMats(ifstream * file);

        // Load a texture
        void _loadTexture(string name, unsigned int & texture);

        // This DOFs display list
        void _createDisplayLists();

        // The bounding box for the dof
        void _calculateBoundingBox();
};

