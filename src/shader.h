/**
 * Parse racer shader files. For now it is overly simple and can only read a
 * single layer.
 *
 * Possible bugs / missing features:
 *  * If map is defined before mipmap, it won't be loaded as a mipmap
 *  * getShader assumes only a single '.' in a filename
 *  * getShader assumes the file casing is right
 *  * Shader assumes a gequal alpha function
 *  * Only have clamp to edge for wrapT
 */
#pragma once

#include <string>
#include <map>

using namespace std;

class Shader {
    public:
        Shader(); 
        string name;

        // Texture stuff
        unsigned int texEnv;
        bool isMipmap;
        unsigned int textureMap;
        string textureMapPath;
        unsigned int wrapT;

        // Alpha stuff
        // This assume a gequal function
        bool blend;
        unsigned int alphaFunc;
        bool alphaFuncSet;

        // Static method and members so we have access to shaders from 
        // anywhere
        static Shader * getShader(string name);
        static void parseShaderFile(string file);



    private:
        static map<string, Shader *> _shaders;
};
