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

#include "shader.h"
#include "texture.h"
#include "opengl_state.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <string>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

// Utility function to load vector
template <class T> void parseVector(ifstream * file, T * vector, int length);
// to load string
int parseString(ifstream * file, char * buffer);

// Define the geometry object flags
#define DOF_COLLISION 2
#define DOF_SURFACE 4

class Dof;

// This should either be a struct or have proper encapsulation. Not sure which yet
class Geob {
    public:
        Geob();
        ~Geob();
        unsigned int material;
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
        unsigned int vao;
        unsigned int vertexVBO;
        unsigned int normalVBO;
        unsigned int textureVBO;
        unsigned int indexVBO;
        Dof * dof;

        // Generate the vao
        void generateVAO();

        // Get the shader for this geob, NULL if there isn't one
        Shader * getShader();
};

class Mat {
    public:
        Mat();
        ~Mat();
        bool isTransparent();

        std::string name;
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
        Texture ** textures;

        // This material's shader
        Shader * shader;

        Geob * getGeob(int index);
        int getNGeobs();
};

class Dof {
    public:
        Dof(std::string filePath, int flags, bool perGeobDisplayList = true);
        ~Dof();

        // These haven't been properly implemented so just throw at the moment
        Dof(const Dof & dof);
        Dof & operator=(const Dof & dof);

        // Render the dof
        int render(bool overrideFrustrumTest = false);

        // Return true if one of the materials is transparent
        bool isTransparent();

        boost::ptr_list<Geob> & getGeobs();

        boost::ptr_vector<Mat> & getMats();

        bool isValid; 

        // Return true if this is part of the track surface
        bool isSurface();

        // Set up the material for OpenGL
        void loadMaterial(Mat & mat);

    private:
        std::string _filePath;

        // Geometrical objects
        boost::ptr_list<Geob> geobs;

        // Material objects
        boost::ptr_vector<Mat> mats;

        // Some objects are small enough that all the geobs can be drawn in two display
        // lists, one for transparent and one for non-transparent. This also lets us
        // optimise the OpenGL state changes.
        bool _perGeobDisplayList;
        unsigned int _displayList;

        // Flags
        int _flags;

        // Parse the geobs from the file into Geob objects. This assumes that the file
        // pointer is at the start of GOB1 and will leave the pointer at the end of GEND
        void _parseGeobs(ifstream * file);

        // Parse the mats from the file into Mat objects. Like _parseGeobs, it assumes that
        // the file pointer is at the beginning of the first MAT0 and leaves it after MEND
        void _parseMats(ifstream * file);

        // Load a texture
        void _loadTexture(std::string name, Texture * texture);

        // This DOFs display list
        void _createDisplayLists();
        // Render a geob, will only change material if previous Mat != the current one
        void _renderGeob(Geob & geob);

        // The bounding box for the dof
        void _calculateBoundingBox();

        // Keep track of the opengl state manually
        OpenGLState _renderState;
};

