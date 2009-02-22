#pragma once

#include "lib.h"
#include "matrix.h"
#include <boost/filesystem.hpp>

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include <string>
#include <vector>
#include <list>
#include <map>


/**
  * TODO:
  *     * Clean up faces when object is destroyed
  *     * Add other components to the Material
  *     * Clean up textures when they are loaded
  *     * Convert to use Boost string libraries everywhere
***/

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace fs = boost::filesystem;

struct Material {
    string name;
    GLuint textureMap;
    bool ambientSet;
    bool diffuseSet;
    bool specularSet;
    bool shininessSet;
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess;
};

struct Face {
    vector<GLfloat>* vertices[3];
    vector<GLfloat>* normals[3];
    vector<GLfloat>* textures[3];
    Material* material;
};


class Obj {
    public:
        Obj(const char *filename);
        void render();
        void renderGroup(string group);
        vector<GLfloat> getVertex(const unsigned int index);
        string filename;
        void calculateBounds(Matrix *transformationMatrix, float *bounds);

        // A cache for loaded objects
        static list< Obj* > objectCache;
        static Obj* makeObj(const char* filename);

    private:
        vector< vector<GLfloat>* > _vertices;
        list< vector<GLfloat>* > _verticesList;
        vector< vector<GLfloat>* > _textureCoords;
        vector< vector<GLfloat>* > _normals;
        list< Material * > _materials;
        list< Face * > _faces;

        // obj groups
        map<string, list< Face * > > _groupFaces;
        map<string, unsigned int> _groupDisplayLists;

        void _addVertex(string line);
        void _addTextureCoord(string line);
        void _addNormal(string line);
        void _addFace(string line, Material* material, string group);

        /**
         * Parse a mtllib line in the obj file. Kicks off a MTL file parse. Return
         * NULL if not found.
         */
        void _addMTL(string line);

        // Get a pointer to the named material
        Material* _findMaterial(string name);

        // Initialise a new material
        void _initMaterial(Material * material, string name);

        // Parse a line into a vector of length length
        vector<GLfloat> _parseLine(string line, const unsigned int length, const char seperator=' ');
        vector<string> _splitLine(string line, const unsigned int length, const char seperator=' ');

        // Create the display list
        unsigned int _createDisplayListForGroup(string group);
        void _createDisplayLists();

        // Load a texture
        static void _loadTexture(string texturePath, GLuint & texture);

        // Keep a counter of display lists so that we can create a new one. Ideally there 
        // should be a proper management class for this.
        static unsigned int _nextDisplayList;

};
